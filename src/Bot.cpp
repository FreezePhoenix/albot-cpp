#include <pthread.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include "SocketWrapper.hpp"
#include "HttpWrapper.hpp"
#include "Bot.hpp"

std::mutex _lock;
std::condition_variable_any _cond;

Bot::Bot(void *id) {
	this->info = (CharacterGameInfo*) id;
	this->mLogger = spdlog::stdout_color_mt(this->info->character->name + ":Bot");
}

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
bool Bot::isMoving() { 
	return data.value("moving", false);
}
bool Bot::isAlive() {
	return data.value("rip", false) == false;
}
void Bot::setParty(const nlohmann::json& j) {
	party.update(j);
}

void Bot::onConnect() {
	this->log("Connected!");
	this->stop();
}

void Bot::updateJson(const nlohmann::json& json) {
    this->data.update(json);
}

std::string Bot::getUsername() {
	return this->name;
}

void Bot::log(std::string str) {
	mLogger->info(str);
}