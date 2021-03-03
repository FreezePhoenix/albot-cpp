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
#include "HttpWrapper/HttpWrapper.hpp"
#include "GameInfo/GameInfo.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include "code-base.hpp"
#include <cstdio>

using namespace std;

using namespace rapidjson;

namespace ALBot {

	void* login(void *id) {
		if (!HttpWrapper::login()) {
			exit(1);
		}
		Document config;
		if (!HttpWrapper::getConfig(&config)) {
			exit(1);
		}

		if (config.HasMember("fetch") && config["fetch"].GetBool()) {
			cout << "Instructed to fetch... fetching characters." << endl;
			if (!HttpWrapper::getCharacters()) {
				exit(1);
			} else {
				cout << "Writing characters to file..." << endl;
				Value chars_array(kArrayType);
				for (int i = 0; i < HttpWrapper::chars.size(); i++) {
					HttpWrapper::Character *struct_char = HttpWrapper::chars[i];
					Value _char(kObjectType);
					Value key("name", config.GetAllocator());
					Value name(struct_char->name.c_str(), config.GetAllocator());
					_char.AddMember(key, name, config.GetAllocator());
					key.SetString(StringRef("id"));
					_char.AddMember(key, struct_char->id, config.GetAllocator());
					key.SetString(StringRef("script"));
					_char.AddMember(key, "Example", config.GetAllocator());
					key.SetString(StringRef("server"));
					_char.AddMember(key, "US II", config.GetAllocator());
					chars_array.PushBack(_char, config.GetAllocator());
				}
				config["fetch"].SetBool(false);

				config.AddMember("characters", chars_array, config.GetAllocator());

				FILE *fp = fopen("bot.out.json", "w"); // non-Windows use "w"
				char writeBuffer[65536];
				FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
				Writer<FileWriteStream> writer(os);
				config.Accept(writer);
				fclose(fp);
				cout << "Characters written to file!" << endl;
				exit(0);
			}
		}
		cout << "Processing characters..." << endl;
		if (!HttpWrapper::processCharacters(config["characters"].GetArray())) {
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
		cout << "Loading symbol hello...\n";
		typedef void* (*hello_t)(void*);
		// reset errors
		dlerror();
		hello_t hello = (hello_t) dlsym(handle, "hello_time");

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
		info->server = HttpWrapper::servers[0];

		void *ret;
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
