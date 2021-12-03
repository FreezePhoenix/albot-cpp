#pragma once

#ifndef ALCLIENT_CPP_HPP_
#define ALCLIENT_CPP_HPP_

#include "albot/GameInfo.hpp"
#include "albot/HttpWrapper.hpp"
#include "albot/BotSkeleton.hpp"

class ALClient {
    public:
        static void login();
        static void get_characters();
        static void fetch_game_data();
        static void get_servers();
        static BotSkeleton* start_character(CharacterGameInfo *info);
};

#endif /* ALCLIENT_CPP_HPP_ */