#ifndef ALBOT_GAMEINFO_HPP_
#define ALBOT_GAMEINFO_HPP_

#include "HttpWrapper.hpp"
#include "Enums/MessageEnum.hpp"

struct Message {
	std::string command;
	std::string requester;
	std::string target;
	void* arguments;
};

class CharacterGameInfo {
	public:
		typedef void* (*HANDLER)(Message*);
		HANDLER parent_handler = nullptr;
		HANDLER child_handler = nullptr;
		HttpWrapper::Server* server;
		HttpWrapper::Character* character;
		HttpWrapper::GameData *G;
		std::string auth;
		std::string userId;
};

class ServiceInfo {
	public:
		typedef void* (*HANDLER)(Message*);
		HANDLER parent_handler = nullptr;
		HANDLER child_handler = nullptr;
		HttpWrapper::GameData *G;
};


#endif /* ALBOT_GAMEINFO_HPP_ */
