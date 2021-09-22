#include <condition_variable>
#include <bits/types/FILE.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <mutex>

#include "Enums/ClassEnum.hpp"
#include "SocketWrapper.hpp"
#include "HttpWrapper.hpp"
#include "JsonUtils.hpp"
#include "GameInfo.hpp"
#include "Bot.hpp"

#ifndef CHARACTER_NAME
	#define CHARACTER_NAME	-1
#endif

#ifndef CHARACTER_CLASS
	#define CHARACTER_CLASS -1
#endif

using namespace rapidjson;

class BotImpl: public Bot {
	public:
		SocketWrapper wrapper;
		BotImpl(void *id): Bot(id), wrapper(std::to_string(info->character->id), this->info->server->url, *this) {
			this->name = info->character->name;
			this->id = info->character->id;
		}
		void onConnect() {
			this->log("Connected!?!");
			this->wrapper.emit("say", { {"message", "Hello Adventure Land, this is C++!"} });
		}
		void start() {
			wrapper.connect();
		}
		void stop() {
			wrapper.close();
		}
};

BotImpl *BotInstance;

extern "C" void* init(void* id) {
	BotInstance = new BotImpl(id);
	BotInstance->log("Class: " + ClassEnum::getClassStringInt(CHARACTER_CLASS));
	BotInstance->log("Logging in... ");
	BotInstance->start();
	sleep(5);
	return BotInstance;
}
