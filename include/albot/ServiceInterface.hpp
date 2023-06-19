#pragma once

#ifndef ALBOT_SERVICE_INTERFACE_HPP_
#define ALBOT_SERVICE_INTERFACE_HPP_

#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#include <nlohmann/json.hpp>
#endif


#include "albot/Enums/ClassEnum.hpp"
#include <string>
#include <variant>

class MutableGameData {
	private:
		std::shared_ptr<nlohmann::json> data;
	public:
		MutableGameData(std::string& rawJson) : data(new nlohmann::json()) {
			// Create a new JSON object.
			nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
		}
		MutableGameData(std::istream& rawJson) : data(new nlohmann::json()) {
			// Create a new JSON object.
			nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
		}
		MutableGameData(const MutableGameData& old) : data(old.data) {
		};
		
		nlohmann::json& getData() { return *data; }
		nlohmann::json& operator[](const std::string& key) { return data->operator[](key); }
		nlohmann::json& at(const std::string& key) {
			return data->at(key);
		}

		friend class GameData;
};
class GameData {
	private:
		std::shared_ptr<nlohmann::json> data;
	public:
		bool was_cached = false;
		GameData() : data(new nlohmann::json()) {
		}
		GameData(std::string& rawJson) : data(new nlohmann::json()) {
			was_cached = false;
			nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
		}
		GameData(std::istream& rawJson): data(new nlohmann::json()) {
			was_cached = true;
			nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
		}
		GameData(const GameData& old) : data(old.data) {
			was_cached = true;
		}
		GameData(const MutableGameData& old) : data(old.data) {
			was_cached = false;
		}

		// Utility methods to access the JSON.
		const nlohmann::json& getData() const {
			return *data;
		}
		const nlohmann::json& operator[](const std::string& key) const {
			return data->operator[](key);
		}
		bool contains(const std::string& key) const {
			return data->contains(key);
		}
		const nlohmann::json& at(const std::string& key) const {
			return data->at(key);
		}
};

struct Character {
	std::string name;
	size_t id;
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

template<typename ARGUMENTS, typename RETURN = void>
class ServiceInfo {
	public:
		typedef RETURN (*HANDLER)(const ARGUMENTS&);
		HANDLER child_handler = nullptr;
		GameData* G;
		void (*destructor)() = nullptr;
		~ServiceInfo() {
			if (destructor != nullptr) {
				destructor();
			}
		}
};

template<typename RETURN>
class ServiceInfo<void, RETURN> {
	public:
		typedef RETURN (*HANDLER)(void);
		HANDLER child_handler = nullptr;
		GameData* G;
		void (*destructor)() = nullptr;
		~ServiceInfo() {
			if (destructor != nullptr) {
				destructor();
			}
		}
};

template<>
class ServiceInfo<void, void> {
	public:
		typedef void (*HANDLER)(void);
		HANDLER child_handler = nullptr;
		GameData* G;
		void (*destructor)() = nullptr;
		~ServiceInfo() {
			if (destructor != nullptr) {
				destructor();
			}
		}
};


#endif /* ALBOT_SERVICE_INTERFACE_HPP_ */