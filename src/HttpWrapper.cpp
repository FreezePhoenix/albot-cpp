#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SSLException.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/StreamCopier.h>
#include <sys/stat.h>
#include <Poco/URI.h>
#include <iostream>
#include <fstream>
#include <regex>

#include "MapProcessing/MapProcessing.hpp"
#include "MapProcessing/Objectifier.hpp"
#include "MapProcessing/Writer.hpp"
#include "HttpWrapper.hpp"
#include "JsonUtils.hpp"


HttpWrapper::GameData HttpWrapper::data = HttpWrapper::GameData();
std::string HttpWrapper::password = "";
std::string HttpWrapper::email = "";
std::string HttpWrapper::session_cookie = "";
std::string HttpWrapper::auth = "";
Poco::Net::NameValueCollection HttpWrapper::cookie = Poco::Net::NameValueCollection();
std::vector<HttpWrapper::nameNumberPair*> HttpWrapper::NAME_NUMBER_PAIRS = std::vector<HttpWrapper::nameNumberPair*>();
std::map<std::string, int> HttpWrapper::NAME_TO_NUMBER = std::map<std::string, int>();
std::vector<HttpWrapper::Character*> HttpWrapper::chars = std::vector<HttpWrapper::Character*>();
std::vector<HttpWrapper::Server*> HttpWrapper::servers = std::vector<HttpWrapper::Server*>();
std::string HttpWrapper::userID = "";

bool HttpWrapper::get_cached_game_version(std::string &version) {
    std::ifstream version_file("game_version");
    if(version_file.fail() || !version_file.is_open()) {
        std::cout << "Failed to find local cache version! Continuing with version 0." << std::endl;
        version = "0";
    } else {
        std::getline(version_file, version);
        version_file.close();
        std::cout << "Found cached version: " << version << std::endl;
    }
    return true;        
}

bool HttpWrapper::get_game_version(std::string &version) {
    std::string raw_data;
    std::cout << "Fetching game version..." << std::endl;
    if(HttpWrapper::do_request("https://adventure.land/", &raw_data)) {
        std::regex version_regex = std::regex("data\\.js\\?v=([0-9]+)\"", std::regex::extended);
        std::smatch sm;
        std::regex_search(raw_data, sm, version_regex);
        // std::regex_match(raw_data.cbegin(), raw_data.cend(), sm, version_regex);
        if(sm.size() > 0) {
            version = sm[1];
            std::cout << "Fetched game version: " << version << std::endl;
            return true;
        } else {
            std::cout << "Failed to find version in file! Aborting." << std::endl;
            return false;
        }
    } else {
        std::cout << "Fetching game version failed! Aborting." << std::endl;
        return false;
    }
}
void HttpWrapper::handleGameJson(HttpWrapper::MutableGameData& data) {
    nlohmann::json &geo = data["geometry"];
    for(nlohmann::detail::iter_impl<nlohmann::json> it = geo.begin(); it != geo.end(); it++) {
        if(it.value()["x_lines"].is_array()) {
            MapProcessing::MapInfo* info = MapProcessing::parse_map(it.value());
            info->name = it.key();
            MapProcessing::simplify_lines(info);
            nlohmann::json x_lines(info->x_lines);
            nlohmann::json y_lines(info->y_lines);
            it.value()["x_lines"] = x_lines;
            it.value()["y_lines"] = y_lines;
            Objectifier objectifier(info);
            objectifier.run();
            Writer writer(&objectifier);
            writer.write();
        }
    }
    HttpWrapper::data = data;
}

bool HttpWrapper::get_game_data() {
    std::string raw_data;
    std::string current_version = "",
        cached_version = "";
    if(get_cached_game_version(cached_version) && get_game_version(current_version)) {
        if(cached_version == current_version) {
            std::ifstream cached_file("data.json");
            if(cached_file.fail() || !cached_file.is_open()) {
                std::cout << "Local cache invalid. Fetching." << std::endl;
            } else {
                std::ostringstream string_stream;
                string_stream << cached_file.rdbuf();
                raw_data = string_stream.str();
                HttpWrapper::data = HttpWrapper::GameData(raw_data);
                return true;
            }
            cached_file.close();
        } else {
            std::cout << "Cached game version does not match! Need: " << current_version << " Have: " << cached_version << " Fetching." << std::endl;
        }
        std::ofstream version_cache("game_version");
        version_cache.write(current_version.c_str(), current_version.length());
        if(HttpWrapper::do_request("https://adventure.land/data.js", &raw_data)) {
            std::cout << "Data fetched! Trimming..." << std::endl;
            raw_data = raw_data.substr(6, raw_data.length() - 8);
            std::cout << "Data trimmed! Parsing..." << std::endl;
            HttpWrapper::MutableGameData data = HttpWrapper::MutableGameData(raw_data);
            HttpWrapper::handleGameJson(data);
            raw_data = HttpWrapper::data.getData().dump();
            std::cout << "Data parsed! Writing cache..." << std::endl;
            std::ofstream cache_file("data.json");
            cache_file.write(raw_data.c_str(), raw_data.length());
            cache_file.close();
            return true;
        } else {
            std::cout << "Fetching data failed! Aborting." << std::endl;
            return false;
        };
    } else {
        std::cout << "Error while checking game versions. Aborting." << std::endl;
        return false;
    }
}

bool HttpWrapper::get_config(nlohmann::json &config) {
    std::cout << "Reading config..." << std::endl;
    std::ifstream configfile("bot.json");
    std::string tmp;
    if (configfile.is_open()) {
        Poco::StreamCopier::copyToString(configfile, tmp);
        JsonUtils::strip_comments(&tmp);
        config = nlohmann::json::parse(tmp.c_str());
        if (!config.is_object()) {
            std::cout << "Config file is empty! Aborting." << std::endl;
            return false;
        }
        if (config["run"].is_array()) {

        }
        std::cout << "Config reading success!" << std::endl;
        return true;
    } else {
        std::cout << "Config file does not exist (bot.json). Aborting." << std::endl;
        return false;
    }
}
bool HttpWrapper::do_post(std::string url, std::string args, std::string *str, std::vector<Poco::Net::HTTPCookie> *cookies) {
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1);
    Poco::Net::HTTPResponse response;

    request.setContentLength(args.length());
    if (!session_cookie.empty()) {
        request.setCookies(cookie);
    }
    session.sendRequest(request) << args;

    std::istream &rs = session.receiveResponse(response);
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
        if (str != nullptr) {
            Poco::StreamCopier::copyToString(rs, *str);
            std::cout << "Fetched " << str->length() << " bytes from " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << std::endl;
        } else {
            std::cout << "Connected to " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << std::endl;
        }
        if (cookies != nullptr) {
            response.getCookies(*cookies);
        }
        return true;
    } else {
        std::cout << response.getStatus() << " " << response.getReason() << ")" << std::endl;
        //it went wrong ?
        return false;
    }
}
bool HttpWrapper::do_request(std::string url, std::string *str) {
    Poco::Net::HTTPResponse response;
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
    if (!session_cookie.empty()) {
        request.setCookies(cookie);
    }
    session.sendRequest(request);
    std::istream &rs = session.receiveResponse(response);
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
        if (str != nullptr) {
            Poco::StreamCopier::copyToString(rs, *str);
            std::cout << "Fetched " << str->length() << " bytes from " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << std::endl;
        } else {
            std::cout << "Connected to " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << std::endl;
        }
        return true;
    } else {
        std::cout << response.getStatus() << " " << response.getReason() << ")" << std::endl;
        //it went wrong ?
        return false;
    }
}
bool HttpWrapper::login() {
    std::cout << "Attempting to log in..." << std::endl;
    // Open the .env file.
    std::ifstream envfile(".env");
    try {
        if (envfile.is_open()) {
        // Read the email and password from a .env file...
        std::getline(envfile, HttpWrapper::email);
        size_t pos = 0;
        pos = HttpWrapper::email.find('=');
        HttpWrapper::email = HttpWrapper::email.substr(pos + 1);
        std::getline(envfile, password);
        pos = HttpWrapper::password.find('=');
        HttpWrapper::password = HttpWrapper::password.substr(pos + 1);
        // Attempt to connect to the server. Since we don't need to copy the output,
        // We pass a nullptr for the output. TODO: Get support for HTTP HEADERS verb.
        if (HttpWrapper::do_request("https://adventure.land", nullptr)) {
            std::cout << "Successfully connected to server!" << std::endl;
            std::string args("arguments={\"email\":\"" + email + "\",\"password\":\"" + password + "\",\"only_login\":true}&method=signup_or_login");
            std::vector < Poco::Net::HTTPCookie > cookies;
            // Again, we don't *really* care about the output the server sends us...
            // We just want the cookies.
            if (HttpWrapper::do_post("https://adventure.land/api/signup_or_login", args, nullptr, &cookies)) {
                for (unsigned int i = 0; i < cookies.size(); i++) {
                    Poco::Net::HTTPCookie _cookie = cookies[i];
                    if (_cookie.getName() == "auth") {
                        HttpWrapper::session_cookie = _cookie.getValue();
                        HttpWrapper::cookie.set("auth", session_cookie);
                        size_t pos = 0;
                        pos = session_cookie.find('-');
                        HttpWrapper::userID = HttpWrapper::session_cookie.substr(0, pos);
                        HttpWrapper::auth = HttpWrapper::session_cookie.substr(pos + 1, HttpWrapper::session_cookie.length());
                        std::cout << "Logged in!" << std::endl;
                        return true;
                    }
                }
                std::cout << "Unable to find login auth cookie. Aborting." << std::endl;
                return false;
            }
            std::cout << "Unable to connect to login API. Aborting." << std::endl;
            return false;
        } else {
            std::cout << "Unable to connect to the website. Aborting." << std::endl;
            return false;
        }
    } else {
        std::cout << ".env file does not exist! Aborting." << std::endl;
        return false;
    }
    } catch(Poco::Net::SSLException e) {
        std::cout << e.displayText() << std::endl;
    }
}
bool HttpWrapper::get_characters() {
    std::string out;
    if (HttpWrapper::api_method("servers_and_characters", "{}", &out)) {
        std::cout << "Characters fetched! Processing..." << std::endl;
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
            return HttpWrapper::process_characters(characters);

        } else {
            std::cout << "Server did not send us an array... trying root object instead" << std::endl;
            return HttpWrapper::process_characters(characters);
        }
    } else {
        std::cout << "Failed to fetch characters! Aborting." << std::endl;
        return false;
    }
}
bool HttpWrapper::get_characters_and_servers() {
    std::string out;
    if (HttpWrapper::api_method("servers_and_characters", "{}", &out)) {
        std::cout << "Characters fetched! Processing..." << std::endl;
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
            return HttpWrapper::process_characters(characters);

        } else {
            std::cout << "Server did not send us an array... trying root object instead" << std::endl;
            return HttpWrapper::process_characters(characters);
        }
    } else {
        std::cout << "Failed to fetch characters! Aborting." << std::endl;
        return false;
    }
}
bool HttpWrapper::process_characters(nlohmann::json &chars) {
    try {
        if (chars.is_array()) {
            HttpWrapper::chars.resize(chars.size());
            HttpWrapper::NAME_NUMBER_PAIRS.resize(chars.size());
            for (int i = 0; i < chars.size(); i++) {
                HttpWrapper::NAME_NUMBER_PAIRS[i] = new std::pair<std::string, int>;
                HttpWrapper::nameNumberPair *pair = HttpWrapper::NAME_NUMBER_PAIRS[i];
                HttpWrapper::chars[i] = new HttpWrapper::Character;
                HttpWrapper::Character *character = HttpWrapper::chars[i];
                nlohmann::json character_json = chars[i].get<nlohmann::basic_json<>>();
                pair->first = character->name = character_json["name"].get<std::string>();
                pair->second = i;
                HttpWrapper::NAME_TO_NUMBER.insert(*pair);
                if (character_json["id"].is_string()) {
                    character->id = std::stol(character_json["id"].get<std::string>());
                } else {
                    character->id = character_json["id"].get<long>();
                }
                if (character_json["script"].is_string()) {
                    character->script = character_json["script"].get<std::string>();
                } else {
                    character->script = "Default";
                }
                if(character_json["type"].is_string()) {
                    character->klass = ClassEnum::getClassEnum(character_json["type"].get<std::string>());
                } else {
                    character->klass = ClassEnum::CLASS_UNKOWN;
                }
                if(character_json["enabled"].is_boolean()) {
                    character->enabled = character_json["enabled"].get<bool>();
                } else {
                    character->enabled = false;
                }
                if (character_json["server"].is_string()) {
                    character->server = character_json["server"].get<std::string>();
                }
            }
        } else {
            std::cout << "Characters array was not an array! Aborting." << std::endl;
            return false;
        }
        std::cout << "Characters processed!" << std::endl;
        return true;
    } catch (...) {
        std::cout << "Failed to process characters. Aborting." << std::endl;
        throw;
    }
}
bool HttpWrapper::get_servers() {
    std::string out;
    if (HttpWrapper::api_method("servers_and_characters", "{}", &out)) {
        std::cout << "Servers fetched! Processing..." << std::endl;
        nlohmann::json servers = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (servers.is_array()) {
            // document.GetArray()
            servers = servers[0]["servers"].get<nlohmann::basic_json<>>();
            return HttpWrapper::process_servers(servers);
        } else {
            std::cout << "Server did not send us an array... trying root object instead" << std::endl;
            return HttpWrapper::process_servers(servers);
        }
        return true;
    } else {
        std::cout << "Failed to fetch servers! Aborting." << std::endl;
        return false;
    }
}
bool HttpWrapper::process_servers(nlohmann::json &servers) {
    try {
        if (servers.is_array()) {
            HttpWrapper::servers.resize(servers.size());
            for (int i = 0; i < servers.size(); i++) {
                HttpWrapper::servers[i] = new HttpWrapper::Server;
                nlohmann::json server = servers[i].get<nlohmann::basic_json<>>();
                HttpWrapper::servers[i]->identifier = server["name"].get<std::string>();
                HttpWrapper::servers[i]->region = server["region"].get<std::string>();
                HttpWrapper::servers[i]->port = server["port"].get<int>();
                HttpWrapper::servers[i]->ip = server["addr"].get<std::string>();
                HttpWrapper::servers[i]->url = server["addr"].get<std::string>() + ":" + std::to_string(server["port"].get<int>());
                HttpWrapper::servers[i]->fullName = server["region"].get<std::string>() + " " + server["name"].get<std::string>();
            }
        } else {
            std::cout << "Servers array was not an array! Aborting." << std::endl;
            return false;
        }
        std::cout << "Servers processed!" << std::endl;
        return true;
    } catch (...) {
        std::cout << "Failed to process servers. Aborting." << std::endl;
        throw;
    }
}
bool HttpWrapper::api_method(std::string method, std::string args, std::string *str) {
    std::string args_string = "arguments=" + args + "&method=" + method;
    return HttpWrapper::do_post("https://adventure.land/api/" + method, args_string, str);
}