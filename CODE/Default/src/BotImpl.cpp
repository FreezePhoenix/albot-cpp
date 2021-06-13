#include <condition_variable>
#include <bits/types/FILE.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <mutex>

#include "../../../src/Enums/ClassEnum.hpp"
#include "../../../src/SocketWrapper.hpp"
#include "../../../src/HttpWrapper.hpp"
#include "../../../src/JsonUtils.hpp"
#include "../../../src/GameInfo.hpp"
#include "../../../src/Bot.hpp"

#if CHARACTER_CLASS + 'a' == 'a'
	#define CHARACTER_CLASS 0
#endif

#ifndef CHARACTER_NAME
	#define CHARACTER_NAME	0
#endif

#ifndef CHARACTER_CLASS
	#define CHARACTER_CLASS 0
#endif

using namespace rapidjson;

class BotImpl: public Bot {
	private:
		SocketWrapper wrapper;
	public:
		BotImpl(void *id): Bot(id), wrapper(std::to_string(info->character->id), this->info->server->url, *this) {
			this->name = info->character->name;
			this->id = info->character->id;
		}
		void onConnect() {
			this->log("Connected!?!");
			this->wrapper.emit("say", {{"message", "Hello Adventure Land, this is C++!"}});
		}
		void start() {
			wrapper.connect();
		}
		void stop() {
			wrapper.close();
		}
};

BotImpl *BotInstance;

extern "C" void* init(void *id) {
	BotInstance = new BotImpl(id);
	BotInstance->log("Class: " + ClassEnum::getClassStringInt(CHARACTER_CLASS));
	BotInstance->log("Logging in... ");
	BotInstance->start();
	sleep(5000);
	return BotInstance;
}
