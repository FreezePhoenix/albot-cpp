//============================================================================
// Name        : LinuxYay.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


#include <nlohmann/json.hpp>
#include <pthread.h>
#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <iomanip>
#include <fstream>

#include "HttpWrapper.hpp"
#include "GameInfo.hpp"
#include "Bot.hpp"
#include "Utils/DoubleLinkedList.hpp"

namespace ALBot {
	inline std::string NULL_PIPE_ALL = " > /dev/null 2> /dev/null";
	inline std::string NULL_PIPE_ERR = " 2> /dev/null";
	void clean_code() {
		system("rm CODE/*.so");
	}
	void build_code(std::string name, std::string char_name, ClassEnum::CLASS klass) {
		std::string FOLDER = "CODE/" + name;
		std::string CMAKE = "cmake -DCHARACTER_NAME=" + std::to_string(HttpWrapper::NAME_TO_NUMBER[char_name]) + " -DCHARACTER_CLASS=" + std::to_string(klass) + " -S " + FOLDER + "/. -B " + FOLDER + "/.";
		std::string MAKE = "make --quiet -C " + FOLDER + "/. -j12";
		std::string CP = "cp " + FOLDER + "/lib" + name + ".so CODE/" + char_name + ".so" + NULL_PIPE_ERR;
		std::cout << "Running CMake on: CODE/" << name << std::endl;
		system(CMAKE.c_str());
		std::cout << "Finished. Compiling..." << std::endl;
		system(MAKE.c_str());
		std::cout << "Finished. Copying..." << std::endl;
		system(CP.c_str());
		std::cout << "Finished." << std::endl;
	}
	void start_character(int index) {
		build_code(HttpWrapper::chars[index]->script, HttpWrapper::chars[index]->name, HttpWrapper::chars[index]->klass);
		pthread_t bot_thread;
		GameInfo *info = new GameInfo;
		int server_index = 0;
		for(int i = 0; i < HttpWrapper::servers.size(); i++) {
			HttpWrapper::Server* server = HttpWrapper::servers[i];
			if(server->fullName == HttpWrapper::chars[index]->server) {
				server_index = i;
				std::cout << "Server found!" << std::endl;
				break;
			}
		}
		info->server = HttpWrapper::servers[server_index];
		info->character = HttpWrapper::chars[index];
		info->auth = HttpWrapper::auth;
		info->userId = HttpWrapper::userID;
		info->G = &HttpWrapper::data;
		std::string file = "CODE/" + HttpWrapper::chars[index]->name + ".so";
		void *handle = dlopen(file.c_str(), RTLD_LAZY);
		if (!handle) {
			std::cerr << "Cannot open library: " << dlerror() << std::endl;
			pthread_exit((void*) 1);
		}
		// load the symbol
		typedef void* (*init_t)(void*);
		// reset errors
		dlerror();
		init_t hello = (init_t) dlsym(handle, "init");

		const char *dlsym_error = dlerror();

		if (dlsym_error) {
			std::cerr << "Cannot load symbol 'hello': " << dlsym_error << std::endl;
			dlclose(handle);
			pthread_exit((void*) 1);
		}
		void *ret;
		pthread_create(&bot_thread, NULL, hello, (void*) info);
		pthread_join(bot_thread, &ret);
		pthread_exit(0);
	}
	
	void* login(void *id) {
		if (!HttpWrapper::login()) {
			exit(1);
		}
		nlohmann::json config;
		if (!HttpWrapper::get_config(config)) {
			exit(1);
		}

		if (!config["fetch"].is_null() && config["fetch"].get<bool>()) {
			std::cout << "Instructed to fetch... fetching characters." << std::endl;
			if (!HttpWrapper::get_characters()) {
				exit(1);
			} else {
				std::cout << "Writing characters to file..." << std::endl;
				nlohmann::json _chars;
				config["characters"] = _chars;

				for (int i = 0; i < HttpWrapper::chars.size(); i++) {
					HttpWrapper::Character *struct_char = HttpWrapper::chars[i];
					nlohmann::json _char;
					_char["name"] = struct_char->name;
					_char["id"] = struct_char->id;
					_char["script"] = "Default";
					_char["server"] = "US II";
					_char["enabled"] = false;
					_char["type"] = ClassEnum::getClassString(struct_char->klass);
					_chars.push_back(_char);
				}

				config["characters"] = _chars;
				config["fetch"] = false;

				std::ofstream o("bot.out.json");
				o << std::setw(4) << config << std::endl;
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
		// std::cout << map["x_lines"].size() << std::endl;
		std::cout << HttpWrapper::data["geometry"]["main"]["x_lines"].size() << std::endl;
		std::cout << HttpWrapper::data["geometry"]["main"]["y_lines"].size() << std::endl;
		clean_code();
		std::shared_ptr<int> a;
		start_character(0);
	}
}

int main() {
	pthread_t login_thread;
		void *ret;
		pthread_create(&login_thread, NULL, ALBot::login, (void*) 0);
		pthread_join(login_thread, &ret);
}
