#ifndef BOTIMPL_LIGHTSOCKET_HPP_
#define BOTIMPL_LIGHTSOCKET_HPP_


#include <string>
#include <nlohmann/json.hpp>

struct LightSocket {
	const std::function<void(const std::string&, std::function<void(const nlohmann::json&)>)> wrapped_register;
	const std::function<void(const std::string&, const nlohmann::json&)> wrapped_emitter;
	const std::function<std::map<std::string, nlohmann::json>& ()> wrapped_entities;
	const std::function<nlohmann::json& ()> wrapped_character;

	void on(const std::string& name, std::function<void(const nlohmann::json&)> handler) const {
		wrapped_register(name, handler);
	}
	void emit(const std::string& name, const nlohmann::json& data) const {
		wrapped_emitter(name, data);
	}
	std::map<std::string, nlohmann::json>& entities() const {
		return wrapped_entities();
	}
	nlohmann::json& character() const {
		return wrapped_character();
	}
};

#endif