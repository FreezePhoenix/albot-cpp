#pragma once
#pragma once
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace std;
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
	static string sessionCookie;
	static NameValueCollection cookie;
	static Character* chars[18];
	static long userID;
	bool static getConfig(Document* json);
	bool static processCharacters(const Value& chars);
	bool static doPost(string url, string args, string* out, vector<HTTPCookie>* cookies = nullptr);
	bool static doRequest(string url, string* str = nullptr);
	bool static login();
	bool static getCharactersAndServers();
	bool static apiMethod(string method, string args, string* out);

};
