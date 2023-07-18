#ifndef ALBOT_BOTSKELETON_HPP_
#define ALBOT_BOTSKELETON_HPP_

#include <atomic>

#include "albot/SocketWrapper.hpp"
#include "albot/Utils/LoopHelper.hpp"
#include "albot/Utils/Timer.hpp"

class BotSkeleton : public Bot {
	protected:
    	Types::TimePoint last; 
		void processInternals();
	public:
		BotSkeleton(const CharacterGameInfo& id);
		LoopHelper loop;
		SocketWrapper wrapper;
		std::atomic<bool> running = true;
		std::atomic<bool> loop_running = false;
		std::thread uvThread;
		void onDisconnect(std::string reason) override;
		void onConnect() override;
		void connect() override;
		void disconnect() override;
		void stop() override;

		nlohmann::json& getUpdateCharacter() override;
		nlohmann::json& getCharacter() override;
};

#endif /* ALBOT_BOTSKELETON_HPP_ */