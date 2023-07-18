#ifndef ALBOT_BOT_HPP_
#define ALBOT_BOT_HPP_

#include <spdlog/logger.h>
#include "albot/GameInfo.hpp"

#define PROXY_GETTER(capName, type) type get##capName();

enum CharacterType { PRIEST = 1, MAGE = 2, RANGER = 3, ROGUE = 4, WARRIOR = 5, PALADIN = 6, MERCHANT = 7 };

class Bot {
	protected:
		std::shared_ptr<spdlog::logger> mLogger;
	public:
		const CharacterGameInfo& info;
		nlohmann::json party;
		std::string name;
		size_t id;
		Bot(const CharacterGameInfo& id);

		bool isMoving();
		bool isAlive();
	    virtual void onPartyRequest(std::string /* name */) {};
	    virtual void onPartyInvite(std::string /* name */) {};
		virtual void onCm(const std::string& /* name */, const nlohmann::json& /* data */) {};
		virtual void onPm(const std::string& /* name */, const std::string& /* message */) {};
		virtual void onChat(const std::string& /* name */, const std::string& /* message */) {};

		virtual void onDisconnect(std::string reason) = 0;
		virtual void onConnect() = 0;
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual void stop() = 0;
		
		void updateCharacter(const nlohmann::json& patch) {
			getUpdateCharacter().update(patch);
		}

		virtual nlohmann::json& getUpdateCharacter() = 0;
		virtual nlohmann::json& getCharacter() = 0;
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

#endif /* ALBOT_BOT_HPP_ */
