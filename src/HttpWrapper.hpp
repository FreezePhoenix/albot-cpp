#ifndef HTTPWRAPPER_HPP_
#define HTTPWRAPPER_HPP_

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SSLException.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "JsonUtils.hpp"

class HttpWrapper {
	private:
		static std::string password;
		static std::string email;
	public:
		class GameData {
			private:
				nlohmann::json data;
			public:
				// This should never be used intentionally!
				GameData() {}
				GameData(std::string& rawJson) : data(nlohmann::json::parse(rawJson)) {}
				GameData(const GameData& old) : data(old.data) {}
				
				const nlohmann::json& getData() const { return data; }

				const nlohmann::json& operator[](const std::string& key) const { return data[key]; }
				const nlohmann::json& at(const std::string& key) const { return data[key]; }
		};
		struct Character {
				std::string name;
				long id;
				std::string script;
				std::string server;
		};
		struct Server {
				std::string region;
				int port;
				bool pvp;
				std::string ip;
				std::string identifier;
				std::string url;
		};
		static HttpWrapper::GameData data;
		static std::string sessionCookie;
		static std::string auth;
		static Poco::Net::NameValueCollection cookie;
		static std::vector<HttpWrapper::Character*> chars;
		static std::vector<HttpWrapper::Server*> servers;
		static std::string userID;
		static nlohmann::json* config;
		bool static getGameData();
		bool static getConfig(nlohmann::json &config);
		bool static doPost(std::string url, std::string args, std::string *str, std::vector<Poco::Net::HTTPCookie> *cookies = nullptr);
		bool static doRequest(std::string url, std::string *str = nullptr);
		bool static login();
		bool static getCharacters();
		bool static getCharactersAndServers();
		bool static processCharacters(nlohmann::json &chars);
		bool static getServers();
		bool static processServers(nlohmann::json &servers);
		bool static apiMethod(std::string method, std::string args, std::string *str);
};

#endif /* HTTPWRAPPER_HPP_ */
