#pragma once

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
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "JsonUtils.hpp"

class HttpWrapper {
	private:
		inline static std::string password = "";
		inline static std::string email = "";
	public:
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
		inline static std::string sessionCookie = "";
		inline static std::string auth = "";
		inline static Poco::Net::NameValueCollection cookie = Poco::Net::NameValueCollection();
		inline static std::vector<HttpWrapper::Character*> chars = std::vector<Character*>();
		inline static std::vector<HttpWrapper::Server*> servers = std::vector<Server*>();
		inline static std::string userID = "";
		bool static getConfig(nlohmann::json &config) {
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
		bool static doPost(std::string url, std::string args, std::string *str, std::vector<Poco::Net::HTTPCookie> *cookies = nullptr) {
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
		bool static doRequest(std::string url, std::string *str = nullptr) {
			Poco::Net::HTTPResponse response;
			Poco::URI uri(url);
			std::string path(uri.getPathAndQuery());
			if (path.empty()) {
				path = "/";
			}
			Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());

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
		bool static login() {

			std::cout << "Attempting to log in..." << std::endl;
			// Open the .env file.
			std::ifstream envfile("./../../.env");
			try {
				if (envfile.is_open()) {
				// Read the email and password from a .env file...
				std::getline(envfile, email);
				std::getline(envfile, password);
				// Attempt to connect to the server. Since we don't need to copy the output,
				// We pass a nullptr for the output. TODO: Get support for HTTP HEADERS verb.
				if (doRequest("https://adventure.land", nullptr)) {
					std::cout << "Successfully connected to server!" << std::endl;
					std::string args("arguments={\"email\":\"" + email + "\",\"password\":\"" + password + "\",\"only_login\":true}&method=signup_or_login");
					std::vector < Poco::Net::HTTPCookie > cookies;
					// Again, we don't *really* care about the output the server sends us...
					// We just want the cookies.
					if (doPost("https://adventure.land/api/signup_or_login", args, nullptr, &cookies)) {
						for (unsigned int i = 0; i < cookies.size(); i++) {
							Poco::Net::HTTPCookie _cookie = cookies[i];
							if (_cookie.getName() == "auth") {
								sessionCookie = _cookie.getValue();
								cookie.set("auth", sessionCookie);
								size_t pos = 0;
								pos = sessionCookie.find('-');
								userID = sessionCookie.substr(0, pos);
								auth = sessionCookie.substr(pos + 1, sessionCookie.length());
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
		bool static getCharacters() {
			std::string out;
			if (apiMethod("servers_and_characters", "{}", &out)) {
				std::cout << "Characters fetched! Processing..." << std::endl;
				nlohmann::json characters = nlohmann::json::parse(out);
				// For some reason we don't just get an object, the API wraps it in an array.
				if (characters.is_array()) {
					characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
					return processCharacters(characters);

				} else {
					std::cout << "Server did not send us an array... trying root object instead" << std::endl;
					return processCharacters(characters);
				}
			} else {
				std::cout << "Failed to fetch characters! Aborting." << std::endl;
				return false;
			}
		}
		bool static getCharactersAndServers() {
			std::string out;
			if (apiMethod("servers_and_characters", "{}", &out)) {
				std::cout << "Characters fetched! Processing..." << std::endl;
				nlohmann::json characters = nlohmann::json::parse(out);
				// For some reason we don't just get an object, the API wraps it in an array.
				if (characters.is_array()) {
					characters = characters[0]["characters"].get<nlohmann::basic_json<>>();
					return processCharacters(characters);

				} else {
					std::cout << "Server did not send us an array... trying root object instead" << std::endl;
					return processCharacters(characters);
				}
			} else {
				std::cout << "Failed to fetch characters! Aborting." << std::endl;
				return false;
			}
		}
		bool static processCharacters(nlohmann::json &chars) {
			try {
				if (chars.is_array()) {
					HttpWrapper::chars.resize(chars.size());
					for (int i = 0; i < chars.size(); i++) {
						HttpWrapper::chars[i] = new Character;
						Character *character = HttpWrapper::chars[i];
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
		bool static getServers() {
			std::string out;
			if (apiMethod("servers_and_characters", "{}", &out)) {
				std::cout << "Servers fetched! Processing..." << std::endl;
				nlohmann::json servers = nlohmann::json::parse(out);
				// For some reason we don't just get an object, the API wraps it in an array.
				if (servers.is_array()) {
					// document.GetArray()
					servers = servers[0]["servers"].get<nlohmann::basic_json<>>();
					return processServers(servers);
				} else {
					std::cout << "Server did not send us an array... trying root object instead" << std::endl;
					return processServers(servers);
				}
				return true;
			} else {
				std::cout << "Failed to fetch servers! Aborting." << std::endl;
				return false;
			}
		}

		bool static processServers(nlohmann::json &servers) {
			try {
				if (servers.is_array()) {
					HttpWrapper::servers.resize(servers.size());
					for (int i = 0; i < servers.size(); i++) {
						HttpWrapper::servers[i] = new Server;
						nlohmann::json server = servers[i].get<nlohmann::basic_json<>>();
						HttpWrapper::servers[i]->identifier = server["name"].get<std::string>();
						HttpWrapper::servers[i]->region = server["region"].get<std::string>();
						HttpWrapper::servers[i]->port = server["port"].get<int>();
						HttpWrapper::servers[i]->ip = server["addr"].get<std::string>();
						HttpWrapper::servers[i]->url = server["addr"].get<std::string>() + ":" + std::to_string(server["port"].get<int>());
						// HttpWrapper::servers[i]->fullName = std::make_pair(server["region"].get<string>(), server["name"].get<string>());
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

		bool static apiMethod(std::string method, std::string args, std::string *str) {
			std::string args_string = "arguments=" + args + "&method=" + method;
			return doPost("https://adventure.land/api/" + method, args_string, str);
		}
};

#endif /* HTTPWRAPPER_HPP_ */
