#ifndef ALBOT_CPP_HPP_
#define ALBOT_CPP_HPP_

#include <dlfcn.h>
#include <memory>
#include <fstream>
#include <future>

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "albot/GameInfo.hpp"
#include <functional>

namespace ALBot {
	extern std::vector<void*> DLHANDLES;
	extern std::map<std::string, ServiceInfo> SERVICE_HANDLERS;
	extern std::map<std::string, CharacterGameInfo> CHARACTER_HANDLERS;
	extern std::vector<std::reference_wrapper<std::thread>> CHARACTER_THREADS;
	extern std::vector<std::thread> SERVICE_THREADS;
	inline std::string NULL_PIPE_OUT = " > /dev/null";
	inline std::string NULL_PIPE_ALL = " > /dev/null 2> /dev/null";
	inline std::string NULL_PIPE_ERR = " 2> /dev/null";
	extern void clean_code();

	extern CharacterGameInfo::HANDLER get_character_handler(const std::string& name);
	extern void ipc_handler(Message message);

	template<typename R = void, typename... Args>
	R invoke_service(const std::string& name, Args&&... args) {
		auto result = std::async<std::function<R(Args...)>, Args...>(SERVICE_HANDLERS.at(name).get_handler<R, Args...>(), std::forward<Args>(args)...);
		result.wait();
		return result.get();
	}

	extern void build_service_code(const std::string& name);

	extern void build_code(const std::string& name, const std::vector<std::string> names, const std::vector<ClassEnum::CLASS>& classes);

	extern void start_service(int index);

	extern void start_character(int index);

	extern void login();
};

#endif
