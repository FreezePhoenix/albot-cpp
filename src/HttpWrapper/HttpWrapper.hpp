#pragma once

#ifndef HTTPWRAPPER_HPP_
#define HTTPWRAPPER_HPP_

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace std;
using namespace nlohmann;
using namespace Poco;
using namespace Poco::Net;
using namespace rapidjson;

class HttpWrapper {
	private:
		static string password;
		static string email;
	public:
		struct Character {
				string name;
				long id;
				string script;
				string server;
		};
		struct Server {
				string region;
				int port;
				bool pvp;
				string ip;
				string identifier;
		};
		static string sessionCookie;
		static string auth;
		static NameValueCollection cookie;
		static vector<Character*> chars;
		static vector<Server*> servers;
		static long userID;
		bool static getConfig(json &config);
		bool static doPost(string url, string args, string *out, vector<HTTPCookie> *cookies = nullptr);
		bool static doRequest(string url, string *str = nullptr);
		bool static login();
		bool static getCharacters();
		bool static getCharactersAndServers();
		bool static processCharacters(json &chars);
		bool static getServers();
		bool static processServers(json &servers);
		bool static apiMethod(string method, string args, string *out);

};

#endif /* HTTPWRAPPER_HPP_ */
