//============================================================================
// Name        : LinuxYay.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "sio_client.h"

#include <chrono>
#include <pthread.h>
#include <ctime>
#include <unistd.h>
#include "HttpWrapper/HttpWrapper.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <cstdio>

using namespace std;

using namespace rapidjson;

namespace ALBot {

	void* login(void* id) {
		if (HttpWrapper::login()) {
			cout << "Logged in!" << endl;
		}
		else {
			cout << "Login failed!" << endl;
			exit(1);
		}
		Document config;
		if(!HttpWrapper::getConfig(&config)) {
			cout << "Config loading failed!" << endl;
			exit(1);
		}
		if(config.HasMember("fetch") && config["fetch"].GetBool()) {
			cout << "Instructed to fetch... fetching characters." << endl;
			HttpWrapper::getCharactersAndServers();
			Value chars_array(kArrayType);
			// HttpWrapper::chars.length == 18

			for(int i = 0; i < 18; i++) {
				if(HttpWrapper::chars[i] != nullptr) {
					HttpWrapper::Character* struct_char = HttpWrapper::chars[i];
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
			}
			config["fetch"].SetBool(false);
			config.AddMember("characters", chars_array, config.GetAllocator());
			// cout << config["characters"][0]["name"].GetString();
			FILE* fp = fopen("bot.out.json", "w"); // non-Windows use "w"
			char writeBuffer[65536];
			FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
			Writer<FileWriteStream> writer(os);
			config.Accept(writer);
			fclose(fp);
			exit(0);
		}
		HttpWrapper::processCharacters(config["characters"].GetArray());
		pthread_exit(0);
	}
	void main() {
		pthread_t login_thread;
		void* ret;
		pthread_create(&login_thread, NULL, login, (void*)1);
		pthread_join(login_thread, &ret);
	}

}

int main() {
	ALBot::main();
}
