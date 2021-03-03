/*
 * GameInfo.hpp
 *
 *  Created on: Mar 3, 2021
 *      Author: linuxlite
 */

#ifndef GAME_INFO_HPP_
#define GAME_INFO_HPP_

#include "/usr/include/c++/9/string"

class HttpWrapper;

using namespace std;

class GameData {

};
class GameInfo {
	public:
		string ip;
		int port;
		long characterID;
		GameData G;
		HttpWrapper *httpwrapper;
};

#endif /* GAME_INFO_HPP_ */
