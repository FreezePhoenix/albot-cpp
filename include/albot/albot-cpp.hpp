//============================================================================
// Name        : LinuxYay.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <dlfcn.h>
#include <memory>
#include <fstream>
#include <future>

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "albot/GameInfo.hpp"

namespace ALBot {
	extern std::map<std::string, ServiceInfo<void, void>*> SERVICE_HANDLERS;
	extern std::map<std::string, CharacterGameInfo*> CHARACTER_HANDLERS;
	extern std::vector<std::thread*> CHARACTER_THREADS;
	extern std::vector<std::thread*> SERVICE_THREADS;
	inline std::string NULL_PIPE_OUT = " > /dev/null";
	inline std::string NULL_PIPE_ALL = " > /dev/null 2> /dev/null";
	inline std::string NULL_PIPE_ERR = " 2> /dev/null";
	extern void clean_code();

	extern CharacterGameInfo::HANDLER get_character_handler(const std::string& name);
	extern void ipc_handler(Message* message);
	
	template<typename ARGUMENTS, typename RESULT = void>
	ServiceInfo<ARGUMENTS, RESULT>::HANDLER get_service_handler(const std::string& name) {
		return (typename ServiceInfo<ARGUMENTS, RESULT>::HANDLER) SERVICE_HANDLERS[name]->child_handler;
	}

	template<typename ARGUMENTS, typename RESULT = void>
	RESULT* invoke_service(const std::string& name, const ARGUMENTS& message) {
		std::future<RESULT*> result = std::async<typename ServiceInfo<ARGUMENTS, RESULT>::HANDLER, const ARGUMENTS*>(get_service_handler<ARGUMENTS, RESULT>(name), &message);
		result.wait();
		return result.get();
	}

	extern void build_service_code(const std::string& name);

	extern void build_character_code(const std::string& name, const std::string& char_name, ClassEnum::CLASS klass);

	extern void start_service(int index);

	extern void start_character(int index);

	extern void login();
};