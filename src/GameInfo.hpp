#pragma once

#ifndef GAMEINFO_HPP_
#define GAMEINFO_HPP_

#include "HttpWrapper.hpp"
#include <rapidjson/document.h>

using namespace rapidjson;

typedef Value GameData;
class GameInfo {
	public:
		HttpWrapper::Server* server;
		HttpWrapper::Character* character;
		HttpWrapper* wrapper;
		GameData *G;
		std::string auth;
		std::string userId;
};


#endif /* GAMEINFO_HPP_ */
