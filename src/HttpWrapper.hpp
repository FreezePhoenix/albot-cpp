#ifndef HTTPWRAPPER_HPP_
#define HTTPWRAPPER_HPP_


#include <Poco/Net/HTTPCookie.h>
#include <nlohmann/json.hpp>
#include "Enums/ClassEnum.hpp"

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
				bool enabled;
				ClassEnum::CLASS klass;
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
				std::string fullName;
		};
		typedef std::pair<std::string, int> nameNumberPair;
		static std::vector<nameNumberPair*> NAME_NUMBER_PAIRS;
		static std::map<std::string, int> NAME_TO_NUMBER;
		static HttpWrapper::GameData data;
		static std::string sessionCookie;
		static std::string auth;
		static Poco::Net::NameValueCollection cookie;
		static std::vector<HttpWrapper::Character*> chars;
		static std::vector<HttpWrapper::Server*> servers;
		static std::string userID;
		static nlohmann::json* config;
		bool static getCachedGameVersion(std::string &version);
		bool static getGameVersion(std::string &version);
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
