#include "alclient-cpp.hpp"
#include <iostream>

void ALClient::login() {
    if(!HttpWrapper::login()) {
        exit(1);
    }
}

void ALClient::fetchGameData() {
    if(!HttpWrapper::getGameData()) {
        exit(1);
    }
}

void ALClient::getServers() {
    if(!HttpWrapper::getServers()) {
        exit(1);
    }
}

BotSkeleton* ALClient::start_character(GameInfo* info) {
    BotSkeleton* bot = new BotSkeleton(info);
    bot->start();
    sleep(5000);
    return bot;
}