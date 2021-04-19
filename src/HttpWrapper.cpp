#include "HttpWrapper.hpp"

HttpWrapper::GameData HttpWrapper::data = HttpWrapper::GameData();
std::string HttpWrapper::password = "";
std::string HttpWrapper::email = "";
std::string HttpWrapper::sessionCookie = "";
std::string HttpWrapper::auth = "";
Poco::Net::NameValueCollection HttpWrapper::cookie = Poco::Net::NameValueCollection();
std::vector<HttpWrapper::Character*> HttpWrapper::chars = std::vector<HttpWrapper::Character*>();
std::vector<HttpWrapper::Server*> HttpWrapper::servers = std::vector<HttpWrapper::Server*>();
std::string HttpWrapper::userID = "";
bool HttpWrapper::getGameData() {
    std::string raw_data; 
    if(HttpWrapper::doRequest("https://adventure.land/data.js", &raw_data)) {
        std::cout << "Data fetched! Trimming..." << std::endl;
        raw_data = raw_data.substr(6, raw_data.length() - 8);
        std::cout << "Data trimmed! Parsing..." << std::endl;
        HttpWrapper::data = HttpWrapper::GameData(raw_data);
        std::cout << "Data parsed!" << std::endl;
        return true;
    } else {
        std::cout << "Fetching data failed! Aborting." << std::endl;
        return false;
    };
}

bool HttpWrapper::getConfig(nlohmann::json &config) {
    std::cout << "Reading config..." << std::endl;
    std::ifstream configfile("bot.json");
    std::string tmp;
    if (configfile.is_open()) {
        Poco::StreamCopier::copyToString(configfile, tmp);
        JsonUtils::stripComments(&tmp);
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
bool HttpWrapper::doPost(std::string url, std::string args, std::string *str, std::vector<Poco::Net::HTTPCookie> *cookies) {
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1);
    Poco::Net::HTTPResponse response;

    request.setContentLength(args.length());
    if (!sessionCookie.empty()) {
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
bool HttpWrapper::doRequest(std::string url, std::string *str) {
    Poco::Net::HTTPResponse response;
    Poco::URI uri(url);
    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }
    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);

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
        std::getline(envfile, email);
        std::getline(envfile, password);
        // Attempt to connect to the server. Since we don't need to copy the output,
        // We pass a nullptr for the output. TODO: Get support for HTTP HEADERS verb.
        if (HttpWrapper::doRequest("https://adventure.land", nullptr)) {
            std::cout << "Successfully connected to server!" << std::endl;
            std::string args("arguments={\"email\":\"" + email + "\",\"password\":\"" + password + "\",\"only_login\":true}&method=signup_or_login");
            std::vector < Poco::Net::HTTPCookie > cookies;
            // Again, we don't *really* care about the output the server sends us...
            // We just want the cookies.
            if (HttpWrapper::doPost("https://adventure.land/api/signup_or_login", args, nullptr, &cookies)) {
                for (unsigned int i = 0; i < cookies.size(); i++) {
                    Poco::Net::HTTPCookie _cookie = cookies[i];
                    if (_cookie.getName() == "auth") {
                        HttpWrapper::sessionCookie = _cookie.getValue();
                        HttpWrapper::cookie.set("auth", sessionCookie);
                        size_t pos = 0;
                        pos = sessionCookie.find('-');
                        HttpWrapper::userID = HttpWrapper::sessionCookie.substr(0, pos);
                        HttpWrapper::auth = HttpWrapper::sessionCookie.substr(pos + 1, HttpWrapper::sessionCookie.length());
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
bool HttpWrapper::getCharacters() {
    std::string out;
    if (HttpWrapper::apiMethod("servers_and_characters", "{}", &out)) {
        std::cout << "Characters fetched! Processing..." << std::endl;
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
            return HttpWrapper::processCharacters(characters);

        } else {
            std::cout << "Server did not send us an array... trying root object instead" << std::endl;
            return HttpWrapper::processCharacters(characters);
        }
    } else {
        std::cout << "Failed to fetch characters! Aborting." << std::endl;
        return false;
    }
}
bool HttpWrapper::getCharactersAndServers() {
    std::string out;
    if (HttpWrapper::apiMethod("servers_and_characters", "{}", &out)) {
        std::cout << "Characters fetched! Processing..." << std::endl;
        nlohmann::json characters = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (characters.is_array()) {
            characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
            return HttpWrapper::processCharacters(characters);

        } else {
            std::cout << "Server did not send us an array... trying root object instead" << std::endl;
            return HttpWrapper::processCharacters(characters);
        }
    } else {
        std::cout << "Failed to fetch characters! Aborting." << std::endl;
        return false;
    }
}
bool HttpWrapper::processCharacters(nlohmann::json &chars) {
    try {
        if (chars.is_array()) {
            HttpWrapper::chars.resize(chars.size());
            for (int i = 0; i < chars.size(); i++) {
                HttpWrapper::chars[i] = new HttpWrapper::Character;
                HttpWrapper::Character *character = HttpWrapper::chars[i];
                nlohmann::json character_json = chars[i].get<nlohmann::basic_json<>>();
                character->name = character_json["name"].get<std::string>();
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
bool HttpWrapper::getServers() {
    std::string out;
    if (HttpWrapper::apiMethod("servers_and_characters", "{}", &out)) {
        std::cout << "Servers fetched! Processing..." << std::endl;
        nlohmann::json servers = nlohmann::json::parse(out);
        // For some reason we don't just get an object, the API wraps it in an array.
        if (servers.is_array()) {
            // document.GetArray()
            servers = servers[0]["servers"].get<nlohmann::basic_json<>>();
            return HttpWrapper::processServers(servers);
        } else {
            std::cout << "Server did not send us an array... trying root object instead" << std::endl;
            return HttpWrapper::processServers(servers);
        }
        return true;
    } else {
        std::cout << "Failed to fetch servers! Aborting." << std::endl;
        return false;
    }
}
bool HttpWrapper::processServers(nlohmann::json &servers) {
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
bool HttpWrapper::apiMethod(std::string method, std::string args, std::string *str) {
    std::string args_string = "arguments=" + args + "&method=" + method;
    return HttpWrapper::doPost("https://adventure.land/api/" + method, args_string, str);
}