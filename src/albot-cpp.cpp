//============================================================================
// Name        : LinuxYay.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <dlfcn.h>
#include <chrono>
#include <pthread.h>
#include <ctime>
#include <unistd.h>
#include "HttpWrapper.hpp"
#include "GameInfo.hpp"
#include "JsonUtils.hpp"
#include <rapidjson/document.h>
#include <sstream>
#include <iomanip>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include "Bot.hpp"
#include <nlohmann/json.hpp>
#include "MapProcessing/MapProcessing.hpp"
#include <cstdio>

namespace ALBot {

	inline std::string NULL_PIPE_ALL = " > /dev/null 2> /dev/null";
	inline std::string NULL_PIPE_ERR = " 2> /dev/null";
	void clean_code() {
		system("rm CODE/*.so");
	}
	void build_code(std::string name, std::string char_name) {
		std::string CMAKE = "cmake CODE/" + name + "/." + NULL_PIPE_ALL;
		std::string MAKE = "make --quiet -C CODE/" + name + "/." + NULL_PIPE_ERR;
		std::string CP = "cp CODE/" + name + "/lib" + name + ".so CODE/" + char_name + ".so" + NULL_PIPE_ALL;
		std::cout << "Running CMake on: CODE/" << name << std::endl;
		system(CMAKE.c_str());
		std::cout << "Finished. Compiling..." << std::endl;
		system(MAKE.c_str());
		std::cout << "Finished. Copying..." << std::endl;
		system(CP.c_str());
		std::cout << "Finished." << std::endl;
	}
	void start_character(int index) {
		
		build_code(HttpWrapper::chars[index]->script, HttpWrapper::chars[index]->name);
		pthread_t bot_thread;
		GameInfo *info = new GameInfo;
		info->server = nullptr;
		info->server = HttpWrapper::servers[7];
		info->character = HttpWrapper::chars[index];
		info->auth = HttpWrapper::auth;
		info->userId = HttpWrapper::userID;
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
		if (!HttpWrapper::getConfig(config)) {
			exit(1);
		}

		if (!config["fetch"].is_null() && config["fetch"].get<bool>()) {
			std::cout << "Instructed to fetch... fetching characters." << std::endl;
			if (!HttpWrapper::getCharacters()) {
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
		if (!HttpWrapper::processCharacters(config["characters"])) {
			exit(1);
		}
		if (!HttpWrapper::getServers()) {

		}
		HttpWrapper::getGameData();
		MapProcessing::MapInfo* map = MapProcessing::parseMap(HttpWrapper::data["geometry"]["main"]);
		MapProcessing::simplify_lines(map);
		clean_code();
		start_character(0);
	}
}

int main() {
	pthread_t login_thread;
		void *ret;
		pthread_create(&login_thread, NULL, ALBot::login, (void*) 0);
		pthread_join(login_thread, &ret);
}
