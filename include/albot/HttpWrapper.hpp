#pragma once

#ifndef ALBOT_HTTPWRAPPER_HPP_
#define ALBOT_HTTPWRAPPER_HPP_

#include <Poco/Net/HTTPCookie.h>

#include <nlohmann/json.hpp>

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "albot/GameInfo.hpp"
#include "albot/ServiceInterface.hpp"

#include <optional>
#include <functional>

class HttpWrapper {
	private:
		static std::shared_ptr<spdlog::logger> mLogger;
		static std::string password;
		static std::string email;
		static int online_version;
	public:
		struct Service {
			std::string name;
			bool enabled;
		};
		static std::string NAME_MACROS;
		typedef std::pair<std::string, int> NameNumberPair;
		static std::map<std::string, int> NAME_TO_NUMBER;
		static GameData data;
		static std::string session_cookie;
		static std::string auth;
		static Poco::Net::NameValueCollection cookie;
		static std::vector<Character> characters;
		static std::vector<HttpWrapper::Service> services;
		static std::vector<Server> servers;
		static std::string userID;
		static nlohmann::json* config;
		// All of the below methods return true if successful, or false otherwise.

		/**
		 * @brief Get the cached game version from a file.
		 *
		 * @param version 
		 * @return true success
		 * @return false failure
		 */
		bool static get_cached_game_version(int& version);
		
		/**
		 * @brief Get the current game version from the server.
		 * 
		 * @param version 
		 * @return true success
		 * @return false failure
		 */
		bool static get_game_version(int& version);

		/**
		 * @brief Get the game data from the server. Uses the cached data if it's valid.
		 * 
		 * @return true 
		 * @return false 
		 */
		bool static get_game_data();
		bool static get_config(nlohmann::json& config);

		/**
		 * @brief Send a HEAD request.
		 * 
		 * @param url 
		 * @param args 
		 * @param cookies Optional cookies.
		 * @return true 
		 * @return false 
		 */
		bool static do_head(const std::string& url, const std::string& args, std::optional<std::reference_wrapper<std::vector<Poco::Net::HTTPCookie>>> cookies = std::nullopt);
		/**
		 * @brief Send a POST request.
		 * 
		 * @param url 
		 * @param args 
		 * @param out String to assign a response to, optionally.
		 * @param cookies Optional cookies.
		 * @return true 
		 * @return false 
		 */
		bool static do_post(const std::string& url, const std::string& args, std::optional<std::reference_wrapper<std::string>> out = std::nullopt, std::optional<std::reference_wrapper<std::vector<Poco::Net::HTTPCookie>>> cookies = std::nullopt);

		/**
		 * @brief Send a GET request.
		 * 
		 * @param url 
		 * @param out String to assign a response to, optionally.
		 * @return true 
		 * @return false 
		 */
		bool static do_request(const std::string& url, std::optional<std::reference_wrapper<std::string>> out = std::nullopt);

		/**
		 * @brief Login to the game.
		 * 
		 * @return true 
		 * @return false 
		 */
		bool static login();
		bool static get_characters();
		bool static get_characters_and_servers();
		bool static process_services(const nlohmann::json& services);
		bool static process_characters(const nlohmann::json& chars);
		bool static get_servers();
		bool static process_servers(const nlohmann::json &servers);
		bool static api_method(const std::string& method, const std::string& args = "{}", std::optional<std::reference_wrapper<std::string>> out = std::nullopt, std::optional<std::reference_wrapper<std::vector<Poco::Net::HTTPCookie>>> cookies = std::nullopt);
		void static handleGameJson(MutableGameData& json);

		int static find_server(const std::string &server_name) {
			for (size_t i = 0; i < HttpWrapper::servers.size(); i++) {
				const Server& server = HttpWrapper::servers[i];
				if (server.fullName == server_name) {
					return i;
				}
			}
			return -1;
		};
};

void from_json(const nlohmann::json& j, Character& value);
void from_json(const nlohmann::json& j, Server& value);
#endif /* ALBOT_HTTPWRAPPER_HPP_ */
