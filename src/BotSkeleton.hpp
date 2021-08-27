#ifndef ALBOT_BOTSKELETON_HPP_
#define ALBOT_BOTSKELETON_HPP_

#include "SocketWrapper.hpp"

class BotSkeleton: public Bot {
	public:
		SocketWrapper wrapper;
		BotSkeleton(GameInfo *id);
		void start();
		void stop();
};

#endif /* ALBOT_BOTSKELETON_HPP_ */