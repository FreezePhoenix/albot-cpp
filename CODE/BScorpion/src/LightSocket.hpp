#ifndef BOTIMPL_LIGHTSOCKET_HPP_
#define BOTIMPL_LIGHTSOCKET_HPP_


#include <string>
#include <nlohmann/json.hpp>

struct LightSocket {
private:
	const std::function<void(std::string, std::function<void(const nlohmann::json&)>)> wrapped_register;
	const std::function<void(std::string, const nlohmann::json&)> wrapped_emitter;
	const std::function<const std::map<std::string, nlohmann::json>& ()> wrapped_getter;
public:
	LightSocket(std::function<void(std::string, std::function<void(const nlohmann::json&)>)> reg, std::function<void(std::string, const nlohmann::json& event)> emit, std::function<const std::map<std::string, nlohmann::json>& ()> getter) : wrapped_register(reg), wrapped_emitter(emit), wrapped_getter(getter) {

	}
	void on(std::string name, std::function<void(const nlohmann::json&)> handler) const {
		wrapped_register(name, handler);
	}
	void emit(std::string name, const nlohmann::json& data) const {
		wrapped_emitter(name, data);
	}
	const std::map<std::string, nlohmann::json>& entities() const {
		return wrapped_getter();
	}
};

#endif