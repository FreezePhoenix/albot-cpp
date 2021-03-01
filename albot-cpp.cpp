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
			config["characters"].SetArray();
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
