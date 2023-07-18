#include <iostream>
#include <string>

#include "albot/Bot.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>

Bot::Bot(const CharacterGameInfo& info): info(info) {
	this->mLogger = spdlog::stdout_color_mt(this->info.character->name + ":Bot");
	this->name = info.character->name;
	this->id = info.character->id;
}

#define PROXY_GETTER_IMPL(cls, name, capName, type)                                                                    \
    type cls::get##capName() { return (type)getCharacter()[name]; }

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

bool Bot::isMoving() { 
	return getCharacter().value("moving", false);
}
bool Bot::isAlive() {
	return getCharacter().value("rip", false) == false;
}
void Bot::setParty(const nlohmann::json& j) {
	party.update(j);
}

void Bot::onDisconnect(std::string reason) {
	this->mLogger->info("Disconnected: {}", reason);
}

void Bot::onConnect() {
	this->mLogger->warn("Connected! Default implementation is to disconnect immediately.");
	this->stop();
}