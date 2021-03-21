
#ifndef GAMEINFO_HPP_
#define GAMEINFO_HPP_

#include "../HttpWrapper/HttpWrapper.hpp"
#include <rapidjson/document.h>

using namespace std;
using namespace rapidjson;

typedef Value GameData;
class GameInfo {
	public:
		HttpWrapper::Server* server;
		HttpWrapper::Character* character;
		HttpWrapper* wrapper;
		GameData *G;
		string auth;
		string userId;
};


#endif /* GAMEINFO_HPP_ */
