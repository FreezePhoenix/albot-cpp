#ifndef ALBOT_GAMEINFO_HPP_
#define ALBOT_GAMEINFO_HPP_

#include "HttpWrapper.hpp"

class GameInfo {
	public:
		HttpWrapper::Server* server;
		HttpWrapper::Character* character;
		HttpWrapper* wrapper;
		HttpWrapper::GameData *G;
		std::string auth;
		std::string userId;
};


#endif /* ALBOT_GAMEINFO_HPP_ */
