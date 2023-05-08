#ifndef ALBOT_SERVICE_INTERFACE_HPP_
#define ALBOT_SERVICE_INTERFACE_HPP_

#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#include <nlohmann/json.hpp>
#endif


#include "albot/Enums/ClassEnum.hpp"
#include <string>

class MutableGameData {
	public:
		nlohmann::json* data;
		MutableGameData(std::string& rawJson) {
			// Create a new JSON object.
			this->data = new nlohmann::json();

			// This mess is to avoid copying the JSON result, and instead constructing it in place.
			nlohmann::detail::parser(nlohmann::detail::input_adapter(rawJson), (nlohmann::json::parser_callback_t) nullptr, true, true).parse(true, *this->data);
		}
		MutableGameData(std::istream& rawJson) {
			// Create a new JSON object.
			this->data = new nlohmann::json();
			
			// This mess is to avoid copying the JSON result, and instead constructing it in place.
			nlohmann::detail::parser(nlohmann::detail::input_adapter(rawJson), (nlohmann::json::parser_callback_t) nullptr, true).parse(true, *this->data);
		}
		MutableGameData(const MutableGameData& old) : data(old.data) {
		};
		
		nlohmann::json& getData() { return *data; }
		nlohmann::json& operator[](const std::string& key) { return data->operator[](key); }
		nlohmann::json& at(const std::string& key) { return data->at(key); }
};
class GameData {
	public:
		bool was_cached = false;
		nlohmann::json* data;
		GameData() : data(nullptr) {
		}
		GameData(std::string& rawJson) {
			was_cached = false;
			// Create a new JSON object.
			this->data = new nlohmann::json();

			// This mess is to avoid copying the JSON result, and instead constructing it in place.
			nlohmann::detail::parser(nlohmann::detail::input_adapter(rawJson), (nlohmann::json::parser_callback_t) nullptr, true, true).parse(true, *this->data);
		}
		GameData(std::istream& rawJson) {
			was_cached = true;
			// Create a new JSON object.
			this->data = new nlohmann::json();
			
			// This mess is to avoid copying the JSON result, and instead constructing it in place.
			nlohmann::detail::parser(nlohmann::detail::input_adapter(rawJson), (nlohmann::json::parser_callback_t) nullptr, true).parse(true, *this->data);
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
		typedef RETURN* (*HANDLER)(ARGUMENTS*);
		HANDLER child_handler = nullptr;
		GameData *G;
};


#endif /* ALBOT_SERVICE_INTERFACE_HPP_ */