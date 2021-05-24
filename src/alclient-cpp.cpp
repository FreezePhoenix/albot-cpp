#include "alclient-cpp.hpp"
#include <iostream>

void ALClient::login() {
    if(!HttpWrapper::login()) {
        exit(1);
    }
}

void ALClient::get_characters() {
    if(!HttpWrapper::get_characters()) {
        exit(1);
    }
}

void ALClient::fetch_game_data() {
    if(!HttpWrapper::get_game_data()) {
        exit(1);
    }
}

void ALClient::get_servers() {
    if(!HttpWrapper::get_servers()) {
        exit(1);
    }
}

BotSkeleton* ALClient::start_character(GameInfo* info) {
    BotSkeleton* bot = new BotSkeleton(info);
    bot->start();
    sleep(5000);
    return bot;
}