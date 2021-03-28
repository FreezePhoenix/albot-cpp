#include <bits/types/FILE.h>
#include <pthread.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include "../../../src/Bot.hpp"
#include "../../../src/HttpWrapper.hpp"
#include "../../../src/SocketWrapper.hpp"
#include "../../../src/GameInfo.hpp"
#include "../../../src/JsonUtils.hpp"

using namespace rapidjson;

class BotImpl: public Bot {
	private:
		SocketWrapper wrapper;
	public:
		BotImpl(void *id): Bot(id), wrapper(std::to_string(info->character->id), this->info->server->url, *this) {
			this->name = info->character->name;
			this->id = info->character->id;
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
	BotInstance->log("Logging in...");
	BotInstance->start();
	sleep(5000);
	return BotInstance;
}
