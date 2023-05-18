#pragma once

#ifndef ALBOT_GAMEINFO_HPP_
#define ALBOT_GAMEINFO_HPP_

#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#include <nlohmann/json.hpp>
#endif

#include "albot/HttpWrapper.hpp"
#include <future>
#include "albot/ServiceInterface.hpp"

struct Message {
	std::string command;
	std::string requester;
	std::string target;
	void* arguments;
};

struct CharacterGameInfo {
	typedef void (*HANDLER)(Message*);
	HANDLER parent_handler = nullptr;
	HANDLER child_handler = nullptr;
	void (*destructor)() = nullptr;
	~CharacterGameInfo() {
		if (destructor != nullptr) {
			destructor();
		}
	}
	Server* server;
	Character* character;
	GameData *G;
	std::string auth;
	std::string userId;
};

#endif /* ALBOT_GAMEINFO_HPP_ */
