#ifndef ALBOT_BOTSKELETON_HPP_
#define ALBOT_BOTSKELETON_HPP_

#include "albot/SocketWrapper.hpp"
#include "albot/Utils/LoopHelper.hpp"
#include "albot/Utils/Timer.hpp"

class BotSkeleton : public Bot {
	protected:
		LoopHelper loop;
    	Types::TimePoint last; 
		void processInternals();
	public:
		SocketWrapper wrapper;
		BotSkeleton(const CharacterGameInfo& id);
		void start();
		void stop();
		bool running = true;
		std::thread uvThread;
		void startUVThread() {
			running = true;
			uvThread = std::thread([this]() {
				while (running) {
					loop.getLoop()->run<uvw::Loop::Mode::ONCE>();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			});
		}
		virtual ~BotSkeleton() {
			if (uvThread.joinable())
				uvThread.join();
		}

};

#endif /* ALBOT_BOTSKELETON_HPP_ */