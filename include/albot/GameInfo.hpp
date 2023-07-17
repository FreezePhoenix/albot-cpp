#pragma once

#ifndef ALBOT_GAMEINFO_HPP_
#define ALBOT_GAMEINFO_HPP_

#include <nlohmann/json.hpp>

#include <future>
#include "albot/ServiceInterface.hpp"

struct Message {
	std::string command;
	std::string requester;
	std::string target;
	void* arguments;
};

struct CharacterGameInfo {
	using HANDLER = std::function<void(Message)>;
	HANDLER parent_handler;
	HANDLER child_handler;
	std::function<void()> destructor;
	~CharacterGameInfo() {
		if (destructor) {
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
