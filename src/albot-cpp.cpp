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
#include "Bot.hpp"
#include "Utils/LoopHelper.hpp"
#include "uvw.hpp"

namespace ALBot {
	//  Private namespace
	namespace {
		static std::shared_ptr<spdlog::logger> mLogger = spdlog::stdout_color_mt<spdlog::async_factory>("ALBotC++");
	}
	std::map<std::string, ServiceInfo*> SERVICE_HANDLERS = std::map<std::string, ServiceInfo*>();
	std::map<std::string, CharacterGameInfo*> CHARACTER_HANDLERS = std::map<std::string, CharacterGameInfo*>();
	std::vector<std::thread*> CHARACTER_THREADS = std::vector<std::thread*>();
	std::vector<std::thread*> SERVICE_THREADS = std::vector<std::thread*>();
	inline std::string NULL_PIPE_OUT = " > /dev/null";
	inline std::string NULL_PIPE_ALL = " > /dev/null 2> /dev/null";
	inline std::string NULL_PIPE_ERR = " 2> /dev/null";
	void clean_code() {
		system("rm CODE/*.so");
		system("rm SERVICES/*.so");
	}

	CharacterGameInfo::HANDLER get_character_handler(const std::string& name) {
		return CHARACTER_HANDLERS.at(name)->child_handler;
	}
	ServiceInfo::HANDLER get_service_handler(const std::string& name) {
		return SERVICE_HANDLERS.at(name)->child_handler;
	}

	void* ipc_handler(Message* message) {
		mLogger->info("RECEIVED {} FROM {}", message->command, message->requester);
		if (message->command == "code_message") {
			mLogger->info("CODE_MESSAGE TARGET: {}", message->target);
			if (CHARACTER_HANDLERS.contains(message->target)) {
				get_character_handler(message->target)(message);
			} else {
				mLogger->warn("TARGET NOT FOUND. USING FALLBACK.");
				get_character_handler(message->requester)(new Message { "code_message_fail", message->requester, message->target, message->arguments });
			}
		} else if (message->command == "service_request") {
			auto result = std::async(std::launch::async, get_service_handler(message->target), message);
			result.wait();
			return result.get();
		}
		return nullptr;
	}

	void build_service_code(const std::string& name) {
		std::string FOLDER = fmt::format("SERVICES/{}", name);
		std::string CMAKE = fmt::format("cmake -DSERVICE_NAME_STRING={} -S {}/. -B {}/. {}", name, FOLDER, FOLDER, NULL_PIPE_OUT);
		std::string MAKE = fmt::format("make --quiet -C {}/. -j8 {}", FOLDER, NULL_PIPE_OUT);
		std::string CP = fmt::format("cp {}/lib{}.so SERVICES/{}.so {}", FOLDER, name, name, NULL_PIPE_OUT);
		mLogger->info("Running CMake on: SERVICES/{}", name);
		system(CMAKE.c_str());
		mLogger->info("Finished. Compiling...");
		system(MAKE.c_str());
		mLogger->info("Finished. Copying...");
		system(CP.c_str());
		mLogger->info("Finished.");
	}

	void build_character_code(const std::string& name, const std::string& char_name, ClassEnum::CLASS klass) {
		std::string FOLDER = "CODE/" + name;
		std::string CMAKE = "cmake -DNAME_DEFINITIONS=\"" + HttpWrapper::NAME_MACROS + "\" -DCHARACTER_NAME_STRING=" + char_name + " -DCHARACTER_NAME=" + std::to_string(HttpWrapper::NAME_TO_NUMBER[char_name]) + " -DCHARACTER_CLASS=" + std::to_string(klass) + " -S " + FOLDER + "/. -B " + FOLDER + "/." + NULL_PIPE_OUT;
		std::string MAKE = "make --quiet -C " + FOLDER + "/. -j8" + NULL_PIPE_OUT;
		std::string CP = "cp " + FOLDER + "/lib" + name + "_" + char_name + ".so CODE/" + char_name + ".so" + NULL_PIPE_OUT;
		mLogger->info("Running CMake on: CODE/{}", name);
		system(CMAKE.c_str());
		mLogger->info("Finished. Compiling...");
		system(MAKE.c_str());
		mLogger->info("Finished. Copying...");
		system(CP.c_str());
		mLogger->info("Finished.");
	}

	void start_service(int index) {
		HttpWrapper::Service& service = HttpWrapper::services[index];
		build_service_code(service.name);
		ServiceInfo* info = new ServiceInfo;
		info->G = &HttpWrapper::data;
		SERVICE_HANDLERS.emplace(service.name, info);
		std::string file = fmt::format("SERVICES/{}.so", service.name);
		void* handle = dlopen(file.c_str(), RTLD_LAZY);
		if (!handle) {
			mLogger->error("Cannot open library: {}", dlerror());
		}

		typedef void* (*init_t)(void*);
		dlerror();
		init_t init = (init_t)dlsym(handle, "init");

		const char *dlsym_error = dlerror();

		if (dlsym_error) {
			mLogger->error("Cannot load symbol 'init': {}", dlsym_error);
			dlclose(handle);
		}

		auto result = std::async(std::launch::async, init, (void*) info);
		result.wait();
		info->child_handler = (ServiceInfo::HANDLER) result.get();
	}

	void start_character(int index) {
		HttpWrapper::Character& character = HttpWrapper::characters[index];
		build_character_code(character.script, character.name, character.klass);
		CharacterGameInfo* info = new CharacterGameInfo;
		int server_index = HttpWrapper::find_server(character.server);
		info->server = HttpWrapper::servers.data() + server_index;
		info->character = HttpWrapper::characters.data() + index;
		info->auth = HttpWrapper::auth;
		info->userId = HttpWrapper::userID;
		info->G = &HttpWrapper::data;
		info->parent_handler = &ipc_handler;
		CHARACTER_HANDLERS.emplace(character.name, info);
		std::string file = "CODE/" + HttpWrapper::characters[index].name + ".so";
		void *handle = dlopen(file.c_str(), RTLD_LAZY);
		if (!handle) {
			mLogger->error("Cannot open library: {}", dlerror());
		}
		// load the symbol
		typedef void* (*init_t)(void*);
		// reset errors
		dlerror();
		init_t init = (init_t) dlsym(handle, "init");

		const char *dlsym_error = dlerror();

		if (dlsym_error) {
			mLogger->error("Cannot load symbol 'init': {}", dlsym_error);
			dlclose(handle);
		}
		std::thread* bot_thread = new std::thread(init, (void*)info);
		CHARACTER_THREADS.push_back(bot_thread);
	}
	
	void login() {
		
		if (!HttpWrapper::login()) {
			exit(1);
		}
		nlohmann::json config = nlohmann::json::object();
		if (!HttpWrapper::get_config(config)) {
			exit(1);
		}

		if (!config["fetch"].is_null() && config["fetch"].get<bool>()) {
			if (!HttpWrapper::get_characters()) {
				exit(1);
			} else {
				mLogger->info("Writing characters to file...");
				nlohmann::json _chars = nlohmann::json::array();

				for (size_t i = 0; i < HttpWrapper::characters.size(); i++) {
					HttpWrapper::Character& struct_char = HttpWrapper::characters[i];
					nlohmann::json _char;
					_char["name"] = struct_char.name;
					_char["id"] = struct_char.id;
					_char["script"] = "Default";
					_char["server"] = "US II";
					_char["enabled"] = false;
					_char["type"] = ClassEnum::getClassString(struct_char.klass);
					_chars.push_back(_char);
				}

				config["characters"] = _chars;
				config["fetch"] = false;
				config["runners"] = nlohmann::json::array();

				std::ofstream o("bot.out.json");
				o << config.dump(4) << std::endl;
				mLogger->info("Characters written to file!");
				exit(0);
			}
		}
		
		if (!HttpWrapper::process_characters(config["characters"])) {
			exit(1);
		}

		if (!HttpWrapper::get_servers()) {

		}
		HttpWrapper::get_game_data();
		clean_code();
		std::vector<size_t> to_run = std::vector<size_t>();
		to_run.reserve(4);
		int merchants_found = 0;
		int fighters_found = 0;
		for (size_t i = 0; i < HttpWrapper::characters.size(); i++) {
			HttpWrapper::Character& character = HttpWrapper::characters[i];
			if (character.enabled) {
				switch (character.klass) {
					case ClassEnum::MERCHANT:
						if (merchants_found < 1) {
							merchants_found++;
							to_run.push_back(i);
						} else {
							mLogger->warn("Merchant slots are full!");
						}
						break;
					default:
						if (fighters_found < 3) {
							fighters_found++;
							to_run.push_back(i);
						} else {
							mLogger->warn("Fighter slots are full!");
						}
				}
			}
		}
		for (size_t i = 0; i < HttpWrapper::services.size(); i++) {
			HttpWrapper::Service& service = HttpWrapper::services[i];
			if (service.enabled) {
				start_service(i);
			}
		}
		for (size_t character : to_run) {
			start_character(character);
		}
		for (size_t i = 0; i < CHARACTER_THREADS.size(); i++) {
			CHARACTER_THREADS[i]->join();
		}
		if (merchants_found == 0 && fighters_found == 0) {
			mLogger->warn("No characters started.");
		}
	}
}

int main() {
	ALBot::login();
}
