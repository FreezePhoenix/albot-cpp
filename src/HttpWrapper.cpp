#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SSLException.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <fstream>
#include <pthread.h>
#include <regex>
#include <fmt/os.h>

#include "MapProcessing/MapProcessing.hpp"
#include "HttpWrapper.hpp"

std::string HttpWrapper::NAME_MACROS = "";
std::shared_ptr<spdlog::logger> HttpWrapper::mLogger = spdlog::stdout_color_mt<spdlog::async_factory>("HttpWrapper");
HttpWrapper::GameData HttpWrapper::data = HttpWrapper::GameData();
int HttpWrapper::online_version = 0;
std::string HttpWrapper::password = "";
std::string HttpWrapper::email = "";
std::string HttpWrapper::session_cookie = "";
std::string HttpWrapper::auth = "";
Poco::Net::NameValueCollection HttpWrapper::cookie = Poco::Net::NameValueCollection();
std::map<std::string, int> HttpWrapper::NAME_TO_NUMBER = std::map<std::string, int>();
std::vector<HttpWrapper::Character> HttpWrapper::characters = std::vector<HttpWrapper::Character>();
std::vector<HttpWrapper::Service> HttpWrapper::services = std::vector<HttpWrapper::Service>();
std::vector<HttpWrapper::Server> HttpWrapper::servers = std::vector<HttpWrapper::Server>();
std::string HttpWrapper::userID = "";

bool HttpWrapper::get_cached_game_version(int& version) {
    std::ifstream version_file("GAME_VERSION");
    if (version_file.fail() || !version_file.is_open()) {
        mLogger->warn("Failed to find local cache version! Assuming outdated cache.");
    } else {
        std::string temp;
        std::getline(version_file, temp);
        version = std::stoi(temp);
        version_file.close();
    }
    return true;        
}

bool HttpWrapper::get_game_version(int &version) {
    std::string raw_data;
    mLogger->info("Fetching current game version...");
    if(HttpWrapper::do_request("https://adventure.land/", &raw_data)) {
        std::regex version_regex = std::regex("data\\.js\\?v=([0-9]+)\"", std::regex::extended);
        std::smatch sm;
        std::regex_search(raw_data, sm, version_regex);
        if(sm.size() > 0) {
            version = std::stoi(sm[1]);
            return true;
        } else {
            mLogger->error("Failed to find version in web page! Aborting.");
            return false;
        }
    } else {
        mLogger->error("Failed to fetch game version! Aborting.");
        return false;
    }
}

void HttpWrapper::handleGameJson(HttpWrapper::MutableGameData& data) {
    nlohmann::json &geo = data["geometry"];
    for (nlohmann::detail::iter_impl<nlohmann::json> it = geo.begin(); it != geo.end(); it++) {
        if (it.value()["placements"].is_array()) {
            geo[it.key()].erase("placements");
        }
        if (it.value()["x_lines"].is_array()) {
            std::shared_ptr<MapProcessing::MapInfo> info = MapProcessing::parse_map(it.value());
            info->name = it.key();
            nlohmann::json& spawns = data["maps"][it.key()]["spawns"];
            info->spawns = std::vector<std::pair<double, double>>();
            info->spawns.reserve(spawns.size());
            for (nlohmann::detail::iter_impl<nlohmann::json> spawn_it = spawns.begin(); spawn_it != spawns.end(); spawn_it++) {
                if(spawn_it.value().is_array()) {
                    info->spawns.push_back(std::pair<double, double>(spawn_it.value()[0].get<double>(), spawn_it.value()[1].get<double>()));
                }
            };
            MapProcessing::simplify_lines(info);
            MapProcessing::process(info);
            geo[info->name]["x_lines"] = info->x_lines;
            geo[info->name]["y_lines"] = info->y_lines;
        }
    }
}

bool HttpWrapper::get_game_data() {
    mLogger->info("Getting game data.");
    std::string raw_data;
    int current_version = HttpWrapper::online_version;
    int cached_version = 0;
    if (true || get_cached_game_version(cached_version)) {
        if (true || cached_version == current_version) {
            std::ifstream cached_file("data.json");
            if (cached_file.fail() || !cached_file.is_open()) {
                mLogger->warn("Local cache invalid. Fetching.");
                cached_file.close();
            } else {
                mLogger->info("Cache version {} is valid.", cached_version);
                HttpWrapper::MutableGameData data = HttpWrapper::MutableGameData(cached_file);
                // In case something changed, just re-process the data.
                handleGameJson(data);
                HttpWrapper::data = data;
                cached_file.close();
                return true;
            }
        } else {
            mLogger->info("Cached game version does not match! Need: {} Have: {}. Fetching.", current_version, cached_version);
        }
        fmt::v8::ostream version_cache = fmt::output_file("GAME_VERSION");
        version_cache.print("{}", current_version);
        version_cache.close();
        if (HttpWrapper::do_request("https://adventure.land/data.js", &raw_data)) {
            mLogger->info("Data fetched! Trimming...");
            raw_data = raw_data.substr(6, raw_data.length() - 8);
            mLogger->info("Data trimmed! Parsing...");
            HttpWrapper::MutableGameData data = HttpWrapper::MutableGameData(raw_data);
            HttpWrapper::handleGameJson(data);
            HttpWrapper::data = data;
            mLogger->info("Data parsed! Writing cache...");
            fmt::v8::ostream cache_file = fmt::output_file("data.json");
            cache_file.print("{}", data.getData().dump());
            cache_file.close();
            mLogger->info("Cache written!");
            return true;
        } else {
            mLogger->error("Fetching data failed! Aborting.");
            return false;
        };
    } else {
        mLogger->error("Error while checking game versions. Aborting.");
        return false;
    }
}

bool HttpWrapper::get_config(nlohmann::json& config) {
    mLogger->info("Reading config...");
    std::ifstream configfile("bot.json");
    std::string tmp;
    if (configfile.is_open()) {
        configfile >> config;
        if (!config.is_object()) {
            mLogger->error("Config file is empty! Aborting.");
            return false;
        }
        if (config["services"].is_array()) {
            mLogger->info("Services detected!");
            process_services(config["services"]);
        } else {
            mLogger->warn("No services detected.");
            config["services"] = nlohmann::json::array();
        }
        mLogger->info("Config reading success!");
        return true;
    } else {
        mLogger->error("Config file (bot.json) does not exist. Aborting.");
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
        request.setCookies(HttpWrapper::cookie);
    }
    session.sendRequest(request) << args;

    std::istream &rs = session.receiveResponse(response);
    if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
        mLogger->info("POST {} (200 OK)", url);
        if (str != nullptr) {
            Poco::StreamCopier::copyToString(rs, *str);
            mLogger->info("Storing {} bytes from response.", str->length());
        }
        if (cookies != nullptr) {
            response.getCookies(*cookies);
        }
        return true;
    } else {
        mLogger->info("POST {} ({} {})", url, response.getStatus(), response.getReason());
        if (str != nullptr) {
            Poco::StreamCopier::copyToString(rs, *str);
            mLogger->info("Storing {} bytes from response.", str->length());
        }
        return false;
    }
}
bool HttpWrapper::do_request(std::string url, std::string* str) {
    Poco::Net::HTTPResponse response;
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
    if (!session_cookie.empty()) {
        request.setCookies(HttpWrapper::cookie);
    }
    session.sendRequest(request);
    std::istream &rs = session.receiveResponse(response);
    if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK) {
        mLogger->info("GET {} (200 OK)", url);
        if (str != nullptr) {
            Poco::StreamCopier::copyToString(rs, *str);
            mLogger->info("Storing {} bytes from response.", str->length());
        }
        return true;
    } else {
        mLogger->info("GET {} ({} {})", url, response.getStatus(), response.getReason());
        if (str != nullptr) {
            Poco::StreamCopier::copyToString(rs, *str);
            mLogger->info("Storing {} bytes from response.", str->length());
        }
        return false;
    }
}
bool HttpWrapper::login() {
    mLogger->info("Attempting to log in...");
    std::ifstream envfile(".env");
    try {
        if (envfile.is_open()) {
            // Read the email and password from a .env file...
            std::getline(envfile, HttpWrapper::email);
            size_t pos = 0;
            pos = HttpWrapper::email.find('=');
            HttpWrapper::email = HttpWrapper::email.substr(pos + 1);
            std::getline(envfile, HttpWrapper::password);
            pos = HttpWrapper::password.find('=');
            HttpWrapper::password = HttpWrapper::password.substr(pos + 1);
            // Attempt to connect to the server. Since we don't need to copy the output,
            // We pass a nullptr for the output. TODO: Get support for HTTP HEADERS verb.
            if (HttpWrapper::get_game_version(HttpWrapper::online_version)) {
                mLogger->info("Successfully connected to server!");
                std::string args("arguments={\"email\":\"" + email + "\",\"password\":\"" + password + "\",\"only_login\":true}&method=signup_or_login");
                std::vector<Poco::Net::HTTPCookie> cookies;
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
                            mLogger->info("Logged in!");
                            return true;
                        }
                    }
                    mLogger->error("Unable to find login auth cookie. Aborting.");
                    return false;
                }
                mLogger->error("Unable to connect to login API. Aborting.");
                return false;
            } else {
                mLogger->error("Unable to connect to the website. Aborting.");
                return false;
            }
        } else {
            mLogger->error(".env file does not exist! Aborting.");
            return false;
        }
    } catch (const Poco::Net::SSLException& e) {
        mLogger->error("CAUGHT EXCEPTION: {}", e.displayText());
        return false;
    }
}
bool HttpWrapper::get_characters() {
    std::string out;
    if (HttpWrapper::api_method("servers_and_characters", "{}", &out)) {
        mLogger->info("Characters fetched! Processing...");
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
            return HttpWrapper::process_characters(characters);

        } else {
            mLogger->warn("Unexpected response from server! Trying root object.");
            return HttpWrapper::process_characters(characters);
        }
    } else {
        mLogger->error("Failed to fetch characters! Aborting.");
        return false;
    }
}
bool HttpWrapper::get_characters_and_servers() {
    std::string out;
    if (HttpWrapper::api_method("servers_and_characters", "{}", &out)) {
        mLogger->info("Characters fetched!");
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
            return HttpWrapper::process_characters(characters);

        } else {
            mLogger->warn("Unexpected response from server! Trying root object.");
            return HttpWrapper::process_characters(characters);
        }
    } else {
        mLogger->error("Failed to fetch characters! Aborting.");
        return false;
    }
}
bool HttpWrapper::process_services(nlohmann::json& service_jsons) {
    try {
        mLogger->info("Processing services...");
        if (service_jsons.is_array()) {
            HttpWrapper::services.resize(service_jsons.size());
            for (size_t i = 0; i < service_jsons.size(); i++) {
                const nlohmann::json& service_json = service_jsons[i];
                HttpWrapper::Service& service = HttpWrapper::services[i];
                service.name = service_json.at("name").get<std::string>();
                service.enabled = service_json.at("enabled").get<bool>();
            }
        } else {
            mLogger->error("Services array was not an array! Aborting.");
            return false;
        }
        mLogger->info("Services processed!");
        return true;
    }
    catch (...) {
        mLogger->error("Failed to process services. Aborting.");
        throw;
    }
}
bool HttpWrapper::process_characters(nlohmann::json& char_jsons) {
    try {
        mLogger->info("Processing characters...");
        if (char_jsons.is_array()) {
            HttpWrapper::characters.resize(char_jsons.size());
            for (unsigned int i = 0; i < char_jsons.size(); i++) {
                const nlohmann::json& character_json = char_jsons[i];
                HttpWrapper::Character& character = HttpWrapper::characters[i];
                character.name = character_json.at("name").get<std::string>();
                if (character_json.at("id").is_number()) {
                    character.id = character_json.at("id").get<long>();
                } else {
                    character.id = std::stol(character_json.at("id").get<std::string>());
                }
                if (character_json.contains("script")) {
                    character.script = character_json.at("script").get<std::string>();
                } else {
                    character.script = "Default";
                }
                character.klass = ClassEnum::getClassEnum(character_json.at("type").get<std::string>());
                if (character_json.contains("enabled")) {
                    character.enabled = character_json.at("enabled").get<bool>();
                } else {
                    character.enabled = false;
                }
                if (character_json.contains("server")) {
                    character.server = character_json.at("server").get<std::string>();
                } else {
                    character.server = "US I";
                }
                HttpWrapper::NAME_TO_NUMBER.emplace(character.name, i);
                std::string UPPER_NAME = character.name;
                std::transform(character.name.begin(), character.name.end(), UPPER_NAME.begin(), ::toupper);
                HttpWrapper::NAME_MACROS += fmt::format("-D{}={} ", UPPER_NAME, i);
            }
        } else {
            mLogger->error("Characters array was not an array! Aborting.");
            return false;
        }
        mLogger->info("Characters processed!");
        return true;
    } catch (...) {
        mLogger->error("Failed to process characters. Aborting.");
        throw;
    }
}
bool HttpWrapper::get_servers() {
    std::string out;
    if (HttpWrapper::api_method("servers_and_characters", "{}", &out)) {
        mLogger->info("Servers fetched! Processing...");
        nlohmann::json servers = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (servers.is_array()) {
            // document.GetArray()
            servers = servers[0]["servers"].get<nlohmann::basic_json<>>();
            return HttpWrapper::process_servers(servers);
        } else {
            mLogger->warn("Server did not send us an array... trying root object instead.");
            return HttpWrapper::process_servers(servers);
        }
        return true;
    } else {
        mLogger->error("Failed to fetch servers! Aborting.");
        return false;
    }
}
bool HttpWrapper::process_servers(nlohmann::json &servers_json) {
    try {
        if (servers_json.is_array()) {
            HttpWrapper::servers.resize(servers_json.size());
            for (size_t i = 0; i < servers_json.size(); i++) {
                const nlohmann::json& server_json = servers_json[i];
                HttpWrapper::Server& server = HttpWrapper::servers[i];
                server.identifier = server_json.at("name").get<std::string>();
                server.region = server_json.at("region").get<std::string>();
                server.port = server_json.at("port").get<int>();
                server.ip = server_json.at("addr").get<std::string>();
                server.url = fmt::format("{}:{}", server.ip, server.port);
                server.fullName = fmt::format("{} {}", server.region, server.identifier);
            }
        } else {
            mLogger->error("Servers array was not an array! Aborting.");
            return false;
        }
        mLogger->info("Servers processed!");
        return true;
    } catch (...) {
        mLogger->error("Failed to process servers. Aborting.");
        throw;
    }
}
void from_json(const nlohmann::json& j, HttpWrapper::Character& value) {
    if (j.contains("name")) {
        value.name = j.at("name").get<std::string>();
    } else {
        fmt::print("Character missing name.\n");
        exit(0);
    }
    if (j.contains("id")) {
		value.id = j.at("id").get<long>();
	} else {
		fmt::print("Character missing ID.\n");
        exit(0);
	}
	if (j.contains("script")) {
		value.script = j.at("script").get<std::string>();
	} else {
		fmt::print("Character missing script.\n");
        exit(0);
	}
	if (j.contains("type")) {
		value.klass = ClassEnum::getClassEnum(j.at("type").get<std::string>());
	} else {
		fmt::print("Character missing class type.\n");
        exit(0);
	}
	if (j.contains("enabled")) {
		value.enabled = j.at("enabled").get<bool>();
	} else {
		fmt::print("Character missing enabled state.\n");
        exit(0);
	}
	if (j.contains("server")) {
		value.server = j.at("server").get<std::string>();
    } else {
        fmt::print("Character missing server.\n");
        exit(0);
    }
}

bool HttpWrapper::api_method(std::string method, std::string args, std::string* str) {
    std::string args_string = "arguments=" + args + "&method=" + method;
    return HttpWrapper::do_post("https://adventure.land/api/" + method, args_string, str);
}