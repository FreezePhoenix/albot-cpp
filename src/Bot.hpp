#ifndef BOT_HPP_
#define BOT_HPP_

#include <pthread.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cstdio>
#include <cstdlib>
#include "GameInfo.hpp"

#define PROXY_GETTER(capName, type) type get##capName();

enum CharacterType { PRIEST = 1, MAGE = 2, RANGER = 3, ROGUE = 4, WARRIOR = 5, PALADIN = 6, MERCHANT = 7 };

class Bot {
	protected:
		std::shared_ptr<spdlog::logger> mLogger;
	public:
		Bot(void *id);
		nlohmann::json data;
		nlohmann::json party;
		GameInfo *info;
		std::string name;
		std::string id;
		void log(std::string str);
		void join_server(std::string str);
		void login();
		bool isMoving();
	    virtual void onPartyRequest(std::string /* name */) {};
	    virtual void onPartyInvite(std::string /* name */) {};
		virtual void onCm(const std::string& /* name */, const nlohmann::json& /* data */) {};
		virtual void onPm(const std::string& /* name */, const std::string& /* message */) {};
		virtual void onChat(const std::string& /* name */, const std::string& /* message */) {};
		virtual void onConnect();
		virtual ~Bot() {};
		void start();
		void stop();
		void updateJson(const nlohmann::json&);
		std::string getUsername();
		nlohmann::json& getRawJson();
		void setParty(const nlohmann::json& j);

		PROXY_GETTER(X, double)
		PROXY_GETTER(Y, double)
		PROXY_GETTER(Hp, int)
		PROXY_GETTER(MaxHp, int)
		PROXY_GETTER(Mp, int)
		PROXY_GETTER(MaxMp, int)
		PROXY_GETTER(Map, std::string)
		PROXY_GETTER(MapId, int)
		PROXY_GETTER(Range, int)
		PROXY_GETTER(CType, std::string)
		PROXY_GETTER(Speed, int)
		PROXY_GETTER(Gold, long long)
		PROXY_GETTER(Id, std::string)
};

#endif /* BOT_HPP_ */
