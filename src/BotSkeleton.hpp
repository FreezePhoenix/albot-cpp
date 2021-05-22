#include <condition_variable>
#include <bits/types/FILE.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <mutex>

#include "Enums/ClassEnum.hpp"
#include "SocketWrapper.hpp"
#include "HttpWrapper.hpp"
#include "JsonUtils.hpp"
#include "GameInfo.hpp"
#include "Bot.hpp"

#ifndef BOTSKELETON_HPP_
#define BOTSKELETON_HPP_

class BotSkeleton: public Bot {
	private:
		SocketWrapper wrapper;
	public:
		BotSkeleton(GameInfo *id);
		void start();
		void stop();
};

#endif /* BOTSKELETON_HPP_ */