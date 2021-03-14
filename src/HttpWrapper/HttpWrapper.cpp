#include "./HttpWrapper.hpp"
#include "../JsonUtils/JsonUtils.hpp"

using namespace std;
using namespace nlohmann;
using namespace Poco;
using namespace Poco::Net;
using namespace rapidjson;

#define GET_STRING(a) string( a .GetString(), a .GetStringLength())

string HttpWrapper::password = "";
string HttpWrapper::auth = "";
string HttpWrapper::email = "";
string HttpWrapper::sessionCookie = "";
vector<HttpWrapper::Character*> HttpWrapper::chars;
vector<HttpWrapper::Server*> HttpWrapper::servers;
NameValueCollection HttpWrapper::cookie = NameValueCollection();
long HttpWrapper::userID = 0;

bool HttpWrapper::doPost(string url, string args, string *str, vector<HTTPCookie> *cookies) {
	URI uri(url);
	string path(uri.getPathAndQuery());
	if (path.empty()) {
		path = "/";
	}
	HTTPClientSession session(uri.getHost(), uri.getPort());

	HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
	HTTPResponse response;

	request.setContentLength(args.length());
	if (!sessionCookie.empty()) {
		request.setCookies(cookie);
	}
	session.sendRequest(request) << args;

	istream &rs = session.receiveResponse(response);
	if (response.getStatus() != HTTPResponse::HTTP_UNAUTHORIZED) {
		if (str != nullptr) {
			StreamCopier::copyToString(rs, *str);
			cout << "Fetched " << str->length() << " bytes from " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << endl;
		} else {
			cout << "Connected to " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << endl;
		}
		if (cookies != nullptr) {
			response.getCookies(*cookies);
		}
		return true;
	} else {
		cout << response.getStatus() << " " << response.getReason() << ")" << endl;
		//it went wrong ?
		return false;
	}
}

bool HttpWrapper::apiMethod(string method, string args, string *str) {
	string args_string = "arguments=" + args + "&method=" + method;
	return doPost("http://adventure.land/api/" + method, args_string, str);
}

bool HttpWrapper::processCharacters(json &chars) {
	try {
		if (chars.is_array()) {
			HttpWrapper::chars.resize(chars.size());
			for (int i = 0; i < chars.size(); i++) {
				HttpWrapper::chars[i] = new Character;
				Character *character = HttpWrapper::chars[i];
				json character_json = chars[i].get<nlohmann::basic_json<>>();
				character->name = character_json["name"].get<string>();
				if (character_json["id"].is_string()) {
					character->id = stol(character_json["id"].get<string>());
				} else {
					character->id = character_json["id"].get<long>();
				}
				if(character_json["script"].is_string()) {
					character->script = character_json["script"].get<string>();
				} else {
					character->script = "Default";
				}
				if(character_json["server"].is_string()) {
					character->server = character_json["server"].get<string>();
				}
			}
		} else {
			cout << "Characters array was not an array! Aborting." << endl;
			return false;
		}
		cout << "Characters processed!" << endl;
		return true;
	} catch (...) {
		cout << "Failed to process characters. Aborting." << endl;
		throw;
	}
}

bool HttpWrapper::getCharacters() {
	string out;
	if (apiMethod("servers_and_characters", "{}", &out)) {
		cout << "Characters fetched! Processing..." << endl;
		json characters = json::parse(out);
		// For some reason we don't just get an object, the API wraps it in an array.
		if (characters.is_array()) {
			characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
			return processCharacters(characters);

		} else {
			cout << "Server did not send us an array... trying root object instead" << endl;
			return processCharacters(characters);
		}
	} else {
		cout << "Failed to fetch characters! Aborting." << endl;
		return false;
	}
}

bool HttpWrapper::getCharactersAndServers() {
	string out;
	if (apiMethod("servers_and_characters", "{}", &out)) {
		cout << "Characters fetched! Processing..." << endl;
		json characters = json::parse(out);
		// For some reason we don't just get an object, the API wraps it in an array.
		if (characters.is_array()) {
			characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
			return processCharacters(characters);

		} else {
			cout << "Server did not send us an array... trying root object instead" << endl;
			return processCharacters(characters);
		}
	} else {
		cout << "Failed to fetch characters! Aborting." << endl;
		return false;
	}
}

bool HttpWrapper::processServers(json &servers) {
	try {
		if (servers.is_array()) {
			HttpWrapper::servers.resize(servers.size());
			for (int i = 0; i < servers.size(); i++) {
				HttpWrapper::servers[i] = new Server;
				json server = servers[i].get<nlohmann::basic_json<>>();
				HttpWrapper::servers[i]->identifier = server["name"].get<string>();
				HttpWrapper::servers[i]->region = server["region"].get<string>();
				HttpWrapper::servers[i]->port = server["port"].get<int>();
				HttpWrapper::servers[i]->ip = server["addr"].get<string>();
			}
		} else {
			cout << "Servers array was not an array! Aborting." << endl;
			return false;
		}
		cout << "Servers processed!" << endl;
		return true;
	} catch (...) {
		cout << "Failed to process servers. Aborting." << endl;
		throw;
	}
}

bool HttpWrapper::getServers() {
	string out;
	if (apiMethod("servers_and_characters", "{}", &out)) {

		cout << out << endl;
		cout << "Servers fetched! Processing..." << endl;
		json servers = json::parse(out);
		// For some reason we don't just get an object, the API wraps it in an array.
		if (servers.is_array()) {
			// document.GetArray()
			servers = servers[0]["servers"].get<nlohmann::basic_json<>>();
			return processServers(servers);
		} else {
			cout << "Server did not send us an array... trying root object instead" << endl;
			return processServers(servers);
		}
		return true;
	} else {
		cout << "Failed to fetch servers! Aborting." << endl;
		return false;
	}
}

bool HttpWrapper::doRequest(string url, string *str) {
	HTTPResponse response;
	URI uri(url);
	std::string path(uri.getPathAndQuery());
	if (path.empty()) {
		path = "/";
	}
	HTTPClientSession session(uri.getHost(), uri.getPort());

	HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);

	session.sendRequest(request);
	istream &rs = session.receiveResponse(response);
	if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
		if (str != nullptr) {
			StreamCopier::copyToString(rs, *str);
			cout << "Fetched " << str->length() << " bytes from " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << endl;
		} else {
			cout << "Connected to " << url << " (" << response.getStatus() << " " << response.getReason() << ")" << endl;
		}
		return true;
	} else {
		cout << response.getStatus() << " " << response.getReason() << ")" << endl;
		//it went wrong ?
		return false;
	}
}

bool HttpWrapper::getConfig(json &config) {
	cout << "Reading config..." << endl;
	ifstream configfile("bot.json");
	string tmp;
	if (configfile.is_open()) {
		StreamCopier::copyToString(configfile, tmp);
		JsonUtils::stripComments(&tmp);
		config = json::parse(tmp.c_str());
		if (!config.is_object()) {
			cout << "Config file is empty! Aborting." << endl;
			return false;
		}
		if(config["run"].is_array()) {

		}
		cout << "Config reading success!" << endl;
		return true;
	} else {
		cout << "Config file does not exist (bot.json). Aborting." << endl;
		return false;
	}
}

bool HttpWrapper::login() {

	cout << "Attempting to log in..." << endl;
	// Open the .env file.
	ifstream envfile("./../../.env");
	if (envfile.is_open()) {
		// Read the email and password from a .env file...
		getline(envfile, email);
		getline(envfile, password);
		// Attempt to connect to the server. Since we don't need to copy the output,
		// We pass a nullptr for the output. TODO: Get support for HTTP HEADERS verb.
		if (doRequest("http://adventure.land", nullptr)) {
			cout << "Successfully connected to server!" << endl;
			string args("arguments={\"email\":\"" + email + "\",\"password\":\"" + password + "\",\"only_login\":true}&method=signup_or_login");
			vector<HTTPCookie> cookies;
			// Again, we don't *really* care about the output the server sends us...
			// We just want the cookies.
			if (doPost("http://adventure.land/api/signup_or_login", args, nullptr, &cookies)) {
				for (unsigned int i = 0; i < cookies.size(); i++) {
					HTTPCookie _cookie = cookies[i];
					if (_cookie.getName() == "auth") {
						sessionCookie = _cookie.getValue();
						cookie.set("auth", sessionCookie);
						size_t pos = 0;
						pos = sessionCookie.find('-');
						userID = stol(sessionCookie.substr(0, pos));
						auth = sessionCookie.substr(pos + 1, sessionCookie.length());
						cout << "Logged in!" << endl;
						return true;
					}
				}
				cout << "Unable to find login auth cookie. Aborting." << endl;
				return false;
			}
			cout << "Unable to connect to login API. Aborting." << endl;
			return false;
		} else {
			cout << "Unable to connect to the website. Aborting." << endl;
			return false;
		}
	} else {
		cout << ".env file does not exist! Aborting." << endl;
		return false;
	}
}
