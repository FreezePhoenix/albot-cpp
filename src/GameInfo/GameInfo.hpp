#pragma once
/*
 * GameInfo.hpp
 *
 *  Created on: Mar 3, 2021
 *      Author: linuxlite
 */

#include "../HttpWrapper/HttpWrapper.hpp"
#include <rapidjson/document.h>

using namespace std;
using namespace rapidjson;

typedef Value GameData;
class GameInfo {
	public:
		HttpWrapper::Server* server;
		HttpWrapper::Character* character;
		GameData *G;
		string auth;
		long userId;
};
