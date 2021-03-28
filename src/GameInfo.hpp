#pragma once

#ifndef GAMEINFO_HPP_
#define GAMEINFO_HPP_

#include "HttpWrapper.hpp"
#include <rapidjson/document.h>

using namespace rapidjson;

class GameInfo {
	public:
		HttpWrapper::Server* server;
		HttpWrapper::Character* character;
		HttpWrapper* wrapper;
		HttpWrapper::GameData *G;
		std::string auth;
		std::string userId;
};


#endif /* GAMEINFO_HPP_ */
