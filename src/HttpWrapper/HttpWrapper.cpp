#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <iostream>
#include <fstream>
#include "HttpWrapper.hpp"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace std;
using namespace Poco;
using namespace Poco::Net;
using namespace rapidjson;

string HttpWrapper::password = "";
string HttpWrapper::email = "";
string HttpWrapper::sessionCookie = "";
HttpWrapper::Character *HttpWrapper::chars[18];
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

bool HttpWrapper::processCharacters(const Value &chars) {
	try {
		if (chars.IsArray()) {
			for (SizeType i = 0; i < chars.Size(); i++) {
				HttpWrapper::chars[i] = new Character;
				const Value &_char = chars[i];
				HttpWrapper::chars[i]->name = string(_char["name"].GetString(), _char["name"].GetStringLength());
				if (_char["id"].IsString()) {
					HttpWrapper::chars[i]->id = stol(string(_char["id"].GetString(), _char["id"].GetStringLength()));
				} else {
					HttpWrapper::chars[i]->id = _char["id"].GetInt64();
				}
				HttpWrapper::chars[i]->script = "Default.js";
				HttpWrapper::chars[i]->server = "US II";
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

bool HttpWrapper::getCharactersAndServers() {
	string out;
	if (apiMethod("servers_and_characters", "{}", &out)) {
		cout << "Characters fetched! Processing..." << endl;
		Document document;
		document.Parse(out.c_str());
		// For some reason we don't just get an object, the API wraps it in an array.
		if (document.IsArray()) {
			// document.GetArray()
			const Value &chars = document[0]["characters"].GetArray();
			return processCharacters(chars);
		} else {
			cout << "Server did not send us an array... trying root object instead" << endl;
			return processCharacters(document.GetArray());
		}
	} else {
		cout << "Failed to fetch characters! Aborting." << endl;
		return false;
	}
}

bool HttpWrapper::getServers() {
	string out;
	if(apiMethod("get_servers", "{}", &out)) {
		cout << out << endl;
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

bool HttpWrapper::getConfig(Document *json) {
	cout << "Reading config..." << endl;
	ifstream configfile("bot.json");
	string tmp;
	if (configfile.is_open()) {
		StreamCopier::copyToString(configfile, tmp);
		json->Parse(tmp.c_str());
		if (!json->IsObject()) {
			cout << "Config file is empty! Aborting." << endl;
			return false;
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
