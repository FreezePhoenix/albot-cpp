//============================================================================
// Name        : LinuxYay.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "sio_client.h"
#include <dlfcn.h>
#include <chrono>
#include <pthread.h>
#include <ctime>
#include <unistd.h>
#include "./HttpWrapper/HttpWrapper.hpp"
#include "./GameInfo/GameInfo.hpp"
#include "./JsonUtils/JsonUtils.hpp"
#include <rapidjson/document.h>
#include <sstream>
#include <iomanip>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include "Bot.hpp"
#include <nlohmann/json.hpp>
#include <cstdio>

using namespace std;
using namespace nlohmann;
using namespace rapidjson;

namespace ALBot {

	void* login(void *id) {
		if (!HttpWrapper::login()) {
			exit(1);
		}
		json config;
		if (!HttpWrapper::getConfig(config)) {
			exit(1);
		}

		if (!config["fetch"].is_null() && config["fetch"].get<bool>()) {
			cout << "Instructed to fetch... fetching characters." << endl;
			if (!HttpWrapper::getCharacters()) {
				exit(1);
			} else {
				cout << "Writing characters to file..." << endl;
				json _chars;
				config["characters"] = _chars;
				for (int i = 0; i < HttpWrapper::chars.size(); i++) {
					HttpWrapper::Character *struct_char = HttpWrapper::chars[i];
					json _char;
					_char["name"] = struct_char->name;
					_char["id"] = struct_char->id;
					_char["script"] = "Example";
					_char["server"] = "US II";
					_chars.push_back(_char);
				}

				config["characters"] = _chars;
				config["fetch"] = false;

				std::ofstream o("bot.out.json");
				o << setw(4) << config << std::endl;
				cout << "Characters written to file!" << endl;
				exit(0);
			}
		}
		cout << "Processing characters..." << endl;
		if (!HttpWrapper::processCharacters(config["characters"])) {
			exit(1);
		}
		if (!HttpWrapper::getServers()) {

		}
		void *handle = dlopen("./libalbot-code.so", RTLD_LAZY);
		if (!handle) {
			cerr << "Cannot open library: " << dlerror() << '\n';
			pthread_exit((void*) 1);
		}
		// load the symbol
		typedef void* (*init_t)(void*);
		// reset errors
		dlerror();
		init_t hello = (init_t) dlsym(handle, "init");

		const char *dlsym_error = dlerror();

		if (dlsym_error) {
			cerr << "Cannot load symbol 'hello': " << dlsym_error << '\n';
			dlclose(handle);
			pthread_exit((void*) 1);
		}
		// use it to do the calculation
		cout << "Calling hello...\n";
		pthread_t bot_thread;
		GameInfo *info = new GameInfo;
		info->server = nullptr;
		cout << "uhoh" << endl;
		info->server = HttpWrapper::servers[7];
		cout << "What do we have here?" << endl;
		info->character = HttpWrapper::chars[0];
		info->auth = HttpWrapper::auth;
		info->userId = HttpWrapper::userID;
		void *ret;
		cout << "It can't be, can it?" << endl;
		pthread_create(&bot_thread, NULL, hello, (void*) info);
		pthread_join(bot_thread, &ret);
		pthread_exit(0);
	}

	void main() {
		pthread_t login_thread;
		void *ret;
		pthread_create(&login_thread, NULL, login, (void*) 0);
		pthread_join(login_thread, &ret);
	}

}

int main() {
	ALBot::main();
}
