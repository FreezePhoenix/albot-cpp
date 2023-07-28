#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SSLException.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <fstream>
#include <pthread.h>
#include <regex>
#include <ranges>
#include <fmt/os.h>

#include "albot/MapProcessing/MapProcessing.hpp"
#include "albot/HttpWrapper.hpp"

std::string HttpWrapper::NAME_MACROS = "";
nlohmann::json* HttpWrapper::config = nullptr;
std::shared_ptr<spdlog::logger> HttpWrapper::mLogger = spdlog::stdout_color_mt("HttpWrapper");
GameData HttpWrapper::data = GameData();
int HttpWrapper::online_version = 0;
std::string HttpWrapper::password = "";
std::string HttpWrapper::email = "";
std::string HttpWrapper::session_cookie = "";
std::string HttpWrapper::auth = "";
Poco::Net::NameValueCollection HttpWrapper::cookie = Poco::Net::NameValueCollection();
std::map<std::string, int> HttpWrapper::NAME_TO_NUMBER = std::map<std::string, int>();
std::vector<Character> HttpWrapper::characters = std::vector<Character>();
std::vector<HttpWrapper::Service> HttpWrapper::services = std::vector<HttpWrapper::Service>();
std::vector<Server> HttpWrapper::servers = std::vector<Server>();
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
    if(HttpWrapper::do_request("https://adventure.land/", raw_data)) {
        std::regex version_regex = std::regex("data\\.js\\?v=([0-9]+)\"", std::regex::extended);
        std::smatch sm;
        if(std::regex_search(raw_data, sm, version_regex)) {
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

void HttpWrapper::handleGameJson(MutableGameData& data) {
    nlohmann::json &geo = data["geometry"];
    double initial = 0;
    double final = 0;
    for (auto& [key, value] : geo.items()) {
        if (value.contains("placements")) {
            value.erase("placements");
        }
        if (value["x_lines"].is_array()) {
            std::shared_ptr<MapProcessing::MapInfo> info = MapProcessing::parse_map(value);
            info->name = key;
            const nlohmann::json& spawns = data["maps"][key]["spawns"];
            info->spawns = std::vector<std::pair<double, double>>();
            info->spawns.reserve(spawns.size());
            for (const nlohmann::json& entry : spawns) {
                info->spawns.emplace_back(entry[0].get<double>(), entry[1].get<double>());
            }
            initial += info->x_lines.size() + info->y_lines.size();
            MapProcessing::simplify_lines(info);
            final += info->x_lines.size() + info->y_lines.size();
            value["x_lines"] = info->x_lines;
            value["y_lines"] = info->y_lines;
        }
    }
    mLogger->info("Simplified Maps. Reduced line count by {}% ({} -> {})", std::trunc((initial - final) / initial * 10000) / 100, initial, final);
}

bool HttpWrapper::get_game_data() {
    mLogger->info("Getting game data.");
    int current_version = HttpWrapper::online_version;
    int cached_version = 0;
    if (get_cached_game_version(cached_version)) {
        if (cached_version == current_version || config->at("servicesOnly").get<bool>()) {
            std::ifstream cached_file("data.json");
            if (cached_file.fail() || !cached_file.is_open()) {
                mLogger->warn("Local cache invalid. Fetching.");
                cached_file.close();
            } else {
                mLogger->info("Cache version {} is valid.", cached_version);
                HttpWrapper::data = cached_file;
                cached_file.close();
                return true;
            }
        } else {
            mLogger->info("Cached game version does not match! Need: {} Have: {}. Fetching.", current_version, cached_version);
        }
        std::string raw_data;
        if (HttpWrapper::do_request("https://adventure.land/data.js", raw_data)) {
            mLogger->info("Data fetched! Trimming...");
            raw_data = raw_data.substr(6, raw_data.length() - 8);
            mLogger->info("Data trimmed! Parsing...");
            MutableGameData data = MutableGameData(raw_data);
            HttpWrapper::handleGameJson(data);
            HttpWrapper::data = data;
            mLogger->info("Data parsed! Writing cache...");
            fmt::v8::ostream cache_file = fmt::output_file("data.json");
            cache_file.print("{}", data.getData().dump());
            cache_file.close();
            fmt::v8::ostream version_cache = fmt::output_file("GAME_VERSION");
            version_cache.print("{}", current_version);
            version_cache.close();
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
        HttpWrapper::config = &config;
        mLogger->info("Config reading success!");
        return true;
    } else {
        mLogger->error("Config file (bot.json) does not exist. Aborting.");
        return false;
    }
}
bool HttpWrapper::do_post(const std::string& url, const std::string& args, std::optional<std::reference_wrapper<std::string>> out, std::optional<std::reference_wrapper<std::vector<Poco::Net::HTTPCookie>>> cookies) {
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1);
    Poco::Net::HTTPResponse response;

    request.setContentLength(args.size());
    if (!session_cookie.empty()) {
        request.setCookies(HttpWrapper::cookie);
    }
    session.sendRequest(request) << args;

    std::istream &rs = session.receiveResponse(response);
    
    mLogger->info("POST {} ({} {})", url, (size_t) response.getStatus(), response.getReason());
    if (out.has_value()) {
        mLogger->info("Storing {} bytes from response.", Poco::StreamCopier::copyToString(rs, out.value().get()));
    }
    if (cookies.has_value()) {
        response.getCookies(cookies.value().get());
    }
    return response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK;
}
bool HttpWrapper::do_head(const std::string& url, const std::string& args, std::optional<std::reference_wrapper<std::vector<Poco::Net::HTTPCookie>>> cookies) {
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_HEAD, path, Poco::Net::HTTPMessage::HTTP_1_1);
    Poco::Net::HTTPResponse response;

    request.setContentLength(args.size());
    if (!session_cookie.empty()) {
        request.setCookies(HttpWrapper::cookie);
    }
    session.sendRequest(request) << args;

    session.receiveResponse(response);
    
    mLogger->info("HEAD {} ({} {})", url, (size_t) response.getStatus(), response.getReason());
    if (cookies.has_value()) {
        response.getCookies(cookies.value().get());
    }
    return response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK;
}
bool HttpWrapper::do_request(const std::string& url, std::optional<std::reference_wrapper<std::string>> out) {
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
    Poco::Net::HTTPResponse response;
    if (!session_cookie.empty()) {
        request.setCookies(HttpWrapper::cookie);
    }
    session.sendRequest(request);
    std::istream& rs = session.receiveResponse(response);
    mLogger->info("GET {} ({} {})", url, (size_t) response.getStatus(), response.getReason());
    if (out.has_value()) {
        mLogger->info("Storing {} bytes from response.", Poco::StreamCopier::copyToString(rs, out.value().get()));
    }
    return response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK;
}
bool HttpWrapper::login() {
    mLogger->info("Attempting to log in...");
    std::ifstream envfile(".env");
    try {
        if (envfile.is_open()) {
            // Read the email and password from a .env file...
            std::getline(envfile, HttpWrapper::email);
            size_t emailPos = HttpWrapper::email.find("email=");
            HttpWrapper::email = HttpWrapper::email.substr(emailPos + 6);
            std::getline(envfile, HttpWrapper::password);
            size_t passwordPos = HttpWrapper::password.find("password=");
            HttpWrapper::password = HttpWrapper::password.substr(passwordPos + 9);
            // Attempt to connect to the server. Since we don't need to copy the output,
            // We pass a nullptr for the output. TODO: Get support for HTTP HEADERS verb.
            if (HttpWrapper::get_game_version(HttpWrapper::online_version)) {
                mLogger->info("Successfully connected to server!");
                std::string args = std::format("{{\"email\":\"{}\",\"password\":\"{}\",\"only_login\":true}}", email, password);
                std::vector<Poco::Net::HTTPCookie> cookies;
                // Again, we don't *really* care about the output the server sends us...
                // We just want the cookies.
                if (HttpWrapper::api_method("signup_or_login", args, std::nullopt, cookies)) {
                    for (size_t i = 0; i < cookies.size(); i++) {
                        Poco::Net::HTTPCookie _cookie = cookies[i];
                        if (_cookie.getName() == "auth") {
                            HttpWrapper::session_cookie = _cookie.getValue();
                            HttpWrapper::cookie.set("auth", session_cookie);
                            size_t pos = session_cookie.find('-');
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
    if (HttpWrapper::api_method("servers_and_characters", "{}", out)) {
        mLogger->info("Characters fetched! Processing...");
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"];
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
    if (HttpWrapper::api_method("servers_and_characters", "{}", out)) {
        mLogger->info("Characters fetched!");
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"];
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
bool HttpWrapper::process_services(const nlohmann::json& service_jsons) {
    try {
        mLogger->info("Processing services...");
        if (service_jsons.is_array()) {
            HttpWrapper::services.reserve(service_jsons.size());
            for (const nlohmann::json& service_json : service_jsons) {
                HttpWrapper::services.emplace_back(service_json["name"].get<std::string>(), service_json["enabled"].get<bool>());   
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
bool HttpWrapper::process_characters(const nlohmann::json& char_jsons) {
    try {
        mLogger->info("Processing characters...");
        if (char_jsons.is_array()) {
            HttpWrapper::NAME_MACROS.reserve(18 * char_jsons.size() + 1);
            HttpWrapper::characters.reserve(char_jsons.size());
            for (size_t i = 0; i < char_jsons.size(); i++) {
                Character& character = HttpWrapper::characters.emplace_back(char_jsons[i]);
                HttpWrapper::NAME_TO_NUMBER.emplace(character.name, i);
                std::string macro_section = std::format("{}={}", character.name, i);
                std::transform(character.name.begin(), character.name.end(), macro_section.begin(), ::toupper);
                HttpWrapper::NAME_MACROS += macro_section;
                if (i < char_jsons.size() - 1) {
                    HttpWrapper::NAME_MACROS += "\\;";
                }
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
    if (HttpWrapper::api_method("servers_and_characters", "{}", out)) {
        mLogger->info("Servers fetched! Processing...");
        nlohmann::json servers = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (servers.is_array()) {
            return HttpWrapper::process_servers(servers[0]["servers"]);
        } else {
            mLogger->warn("Server did not send us an array response! Aborting.");
            return false;
        }
        return true;
    } else {
        mLogger->error("Failed to fetch servers! Aborting.");
        return false;
    }
}

bool HttpWrapper::process_servers(const nlohmann::json& servers_json) {
    try {
        if (servers_json.is_array()) {
            HttpWrapper::servers.reserve(servers_json.size());
            for (size_t i = 0; i < servers_json.size(); i++) {
                HttpWrapper::servers.emplace_back(servers_json[i]);
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

void from_json(const nlohmann::json& character_json, Character& character) {
    character.name = character_json.at("name").get<std::string>();
    if (character_json.at("id").is_number()) {
        character.id = character_json["id"].get<long>();
    } else {
        character.id = std::stol(character_json["id"].get<std::string>());
    }
    character.script = character_json.value("script", "Default");
    character.enabled = character_json.value("enabled", false);
    character.server = character_json.value("server", "US I");
    character.klass = ClassEnum::getClassEnum(character_json.value("type", "unknown"));
}

void from_json(const nlohmann::json& server_json, Server& server) {
    server.identifier = server_json.at("name").get<std::string>();
    server.region = server_json.at("region").get<std::string>();
    server.port = server_json.at("port").get<int>();
    server.ip = server_json.at("addr").get<std::string>();
    server.url = std::format("{}:{}", server.ip, server.port);
    server.fullName = std::format("{} {}", server.region, server.identifier);
}

bool HttpWrapper::api_method(const std::string& method, const std::string& args, std::optional<std::reference_wrapper<std::string>> out, std::optional<std::reference_wrapper<std::vector<Poco::Net::HTTPCookie>>> cookies) {
    return HttpWrapper::do_post(std::format("https://adventure.land/api/{}", method), std::format("arguments={}&method={}", args, method), out, cookies);
}