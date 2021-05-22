#include <condition_variable>
#include <bits/types/FILE.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <mutex>

#include "BotSkeleton.hpp"


BotSkeleton::BotSkeleton(GameInfo *id): Bot(id), wrapper(std::to_string(info->character->id), this->info->server->url, *this) {
    this->name = info->character->name;
    this->id = info->character->id;
}
void BotSkeleton::start() {
    wrapper.connect();
};
void BotSkeleton::stop() {
    wrapper.close();
};