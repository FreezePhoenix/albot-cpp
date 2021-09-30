//============================================================================
// Name        : LinuxYay.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <pthread.h>
#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <fstream>
#include "Bot.hpp"

namespace ALBot {
	std::vector<pthread_t> THREADS = std::vector<pthread_t>();
	inline std::string NULL_PIPE_OUT = " > /dev/null";
	inline std::string NULL_PIPE_ALL = " > /dev/null 2> /dev/null";
	inline std::string NULL_PIPE_ERR = " 2> /dev/null";
	void clean_code() {
		system("rm CODE/*.so");
	}
	void build_code(std::string name, std::string char_name, ClassEnum::CLASS klass) {
		std::string FOLDER = "CODE/" + name;
		std::string CMAKE = "cmake -DNAME_DEFINITIONS=\"" + HttpWrapper::NAME_MACROS + "\" -DCHARACTER_NAME_STRING=" + char_name + " -DCHARACTER_NAME=" + std::to_string(HttpWrapper::NAME_TO_NUMBER[char_name]) + " -DCHARACTER_CLASS=" + std::to_string(klass) + " -S " + FOLDER + "/. -B " + FOLDER + "/." + NULL_PIPE_OUT;
		std::string MAKE = "make --quiet -C " + FOLDER + "/. -j8" + NULL_PIPE_OUT;
		std::string CP = "cp " + FOLDER + "/lib" + name + "_" + char_name + ".so CODE/" + char_name + ".so" + NULL_PIPE_OUT;
		std::cout << "Running CMake on: CODE/" << name << std::endl;
		system(CMAKE.c_str());
		std::cout << "Finished. Compiling..." << std::endl;
		system(MAKE.c_str());
		std::cout << "Finished. Copying..." << std::endl;
		system(CP.c_str());
		std::cout << "Finished." << std::endl;
	}
	void start_character(int index) {
		HttpWrapper::Character& character = HttpWrapper::characters[index];
		build_code(character.script, character.name, character.klass);
		pthread_t bot_thread;
		GameInfo* info = new GameInfo;
		int server_index = 0;
		for(int i = 0; i < HttpWrapper::servers.size(); i++) {
			HttpWrapper::Server& server = HttpWrapper::servers[i];
			if(server.fullName == character.server) {
				server_index = i;
				std::cout << "Server found!" << std::endl;
				break;
			}
		}
		info->server = HttpWrapper::servers.data() + server_index;
		info->character = HttpWrapper::characters.data() + index;
		info->auth = HttpWrapper::auth;
		info->userId = HttpWrapper::userID;
		info->G = &HttpWrapper::data;
		std::string file = "CODE/" + HttpWrapper::characters[index].name + ".so";
		void *handle = dlopen(file.c_str(), RTLD_LAZY);
		if (!handle) {
			std::cerr << "Cannot open library: " << dlerror() << std::endl;
		}
		// load the symbol
		typedef void* (*init_t)(void*);
		// reset errors
		dlerror();
		init_t init = (init_t) dlsym(handle, "init");

		const char *dlsym_error = dlerror();

		if (dlsym_error) {
			std::cerr << "Cannot load symbol 'init': " << dlsym_error << std::endl;
			dlclose(handle);
			pthread_exit((void*)1);
		}
		void *ret;
		pthread_create(&bot_thread, NULL, init, (void*)info);
		THREADS.push_back(bot_thread);
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
			std::cout << "Instructed to fetch... fetching characters." << std::endl;
			if (!HttpWrapper::get_characters()) {
				exit(1);
			} else {
				std::cout << "Writing characters to file..." << std::endl;
				nlohmann::json _chars = nlohmann::json::array();

				for (int i = 0; i < HttpWrapper::characters.size(); i++) {
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

				std::ofstream o("bot.out.json");
				std::cout << "Characters written to file!" << std::endl;
				exit(0);
			}
		}


		std::cout << "Processing characters..." << std::endl;
		if (!HttpWrapper::process_characters(config["characters"])) {
			exit(1);
		}

		if (!HttpWrapper::get_servers()) {

		}
		HttpWrapper::get_game_data();
		clean_code();
		// std::cout << HttpWrapper::characters[0].name << std::endl;
		// start_character(0);
		for (size_t i = 0; i < THREADS.size(); i++) {
			pthread_join(THREADS[i], nullptr);
		}
	}
}

int main() {
	ALBot::login();
}
