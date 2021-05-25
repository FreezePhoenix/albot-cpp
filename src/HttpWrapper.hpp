#pragma once

#ifndef ALBOT_HTTPWRAPPER_HPP_
#define ALBOT_HTTPWRAPPER_HPP_

#include <Poco/Net/HTTPCookie.h>
#include <nlohmann/json.hpp>

#include "Enums/ClassEnum.hpp"

class HttpWrapper {
	private:
		class MutableGameData {
			public:
				nlohmann::json data;
				MutableGameData() {}
				MutableGameData(std::string& rawJson) : data(nlohmann::json::parse(rawJson)) {}
				MutableGameData(const MutableGameData& old) : data(old.data) {}
				
				nlohmann::json& getData() { return data; }

				nlohmann::json& operator[](const std::string& key) { return data[key]; }
				nlohmann::json& at(const std::string& key) { return data[key]; }
		};
		static std::string password;
		static std::string email;
		static void handleGameJson(HttpWrapper::MutableGameData& json);
	public:
		class GameData {
			private:
				nlohmann::json data;
			public:
				// This should never be used intentionally!
				GameData() {}
				GameData(std::string& rawJson) : data(nlohmann::json::parse(rawJson)) {}
				GameData(const GameData& old) : data(old.data) {}
				GameData(const MutableGameData& old) : data(old.data) {}
				
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
		static std::string session_cookie;
		static std::string auth;
		static Poco::Net::NameValueCollection cookie;
		static std::vector<HttpWrapper::Character*> chars;
		static std::vector<HttpWrapper::Server*> servers;
		static std::string userID;
		static nlohmann::json* config;
		bool static get_cached_game_version(std::string &version);
		bool static get_game_version(std::string &version);
		bool static get_game_data();
		bool static get_config(nlohmann::json &config);
		bool static do_post(std::string url, std::string args, std::string *str, std::vector<Poco::Net::HTTPCookie> *cookies = nullptr);
		bool static do_request(std::string url, std::string *str = nullptr);
		bool static login();
		bool static get_characters();
		bool static get_characters_and_servers();
		bool static process_characters(nlohmann::json &chars);
		bool static get_servers();
		bool static process_servers(nlohmann::json &servers);
		bool static api_method(std::string method, std::string args, std::string *str);
};

#endif /* ALBOT_HTTPWRAPPER_HPP_ */
