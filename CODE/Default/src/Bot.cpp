#include "./Bot.hpp"
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
#include "../../../src/HttpWrapper.hpp"
#include "SocketWrapper.hpp"
#include "../../../src/GameInfo.hpp"
#include "./sio_client.h"
#include "../../../src/JsonUtils.hpp"

using namespace std;
using namespace sio;
using namespace rapidjson;

std::mutex _lock;
std::condition_variable_any _cond;

Bot::Bot(void *id) {
	this->info = (GameInfo*) id;
}

class connection_listener {
		sio::client &handler;

	public:

		connection_listener(sio::client &h) : handler(h) {
		}

		void on_connected() {
			_lock.lock();
			_cond.notify_all();
			_lock.unlock();
		}
		void on_close(client::close_reason const &reason) {
			std::cout << "sio closed " << std::endl;
			exit(0);
		}

		void on_fail() {
			std::cout << "sio failed " << std::endl;
			exit(0);
		}
};

#define PROXY_GETTER_IMPL(cls, name, capName, type)                                                                    \
    type cls::get##capName() { return data[name].get<type>(); }

PROXY_GETTER_IMPL(Bot, "x", X, double)
PROXY_GETTER_IMPL(Bot, "y", Y, double)
PROXY_GETTER_IMPL(Bot, "hp", Hp, int)
PROXY_GETTER_IMPL(Bot, "max_hp", MaxHp, int)
PROXY_GETTER_IMPL(Bot, "mp", Mp, int)
PROXY_GETTER_IMPL(Bot, "max_mp", MaxMp, int)
PROXY_GETTER_IMPL(Bot, "map", Map, std::string)
PROXY_GETTER_IMPL(Bot, "m", MapId, int)
PROXY_GETTER_IMPL(Bot, "range", Range, int)
PROXY_GETTER_IMPL(Bot, "ctype", CType, std::string)
PROXY_GETTER_IMPL(Bot, "speed", Speed, int)
PROXY_GETTER_IMPL(Bot, "gold", Gold, long long)
PROXY_GETTER_IMPL(Bot, "id", Id, std::string)


nlohmann::json& Bot::getRawJson() {
	return this->data;
}

void Bot::start() {}

void Bot::stop() {}


void Bot::onConnect() {
	this->log("Connected!");
}

void Bot::updateJson(const nlohmann::json& json) {
    this->data.update(json);
}

string Bot::getUsername() {
	return this->name;
}

void Bot::log(string str) {
	cout << "(" << this->info->character->name << "): " << str << endl;
}

class BotImpl: public Bot {
	private:
		SocketWrapper wrapper;
	public:
		BotImpl(void *id): Bot(id), wrapper(to_string(info->character->id), this->info->server->url, *this) {
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
