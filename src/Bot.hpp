#ifndef BOT_HPP_
#define BOT_HPP_

#include <bits/types/FILE.h>
#include <pthread.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <cstdlib>
#include "GameInfo.hpp"
#include "HttpWrapper.hpp"
#include <iostream>
#include <string>

using namespace rapidjson;

#define PROXY_GETTER(capName, type) type get##capName();

class Bot {
	protected:
		static auto inline const mLogger = spdlog::stdout_color_mt("Bot");;
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
