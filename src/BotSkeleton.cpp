#include <condition_variable>
#include <pthread.h>
#include <iostream>
#include <string>
#include <mutex>

#include "albot/Utils/Timer.hpp"
#include "albot/BotSkeleton.hpp"
#include "albot/MovementMath.hpp"
#include "albot/Utils/ParsingUtils.hpp"

BotSkeleton::BotSkeleton(CharacterGameInfo *id): Bot(id), wrapper(std::to_string(info->character->id), this->info->server->url, *this), loop() {
    this->name = info->character->name;
    this->id = info->character->id;
    loop.setInterval([this](const uvw::TimerEvent&, uvw::TimerHandle&) {
        this->processInternals();
    }, 1000.0 / 60.0); 
}

const Types::TimePoint epoch;

void BotSkeleton::processInternals() {
    if (last == epoch) last = Types::Clock::now();

    std::map<std::string, nlohmann::json> updateEntities;

    {
        // The intermediate map is used to reduce the amount of data races.
        // Now, it's only between the socket, and this function. This specific
        // tiny code snippet uses a mutex, that interacts with the inserting
        // and updating function (entities event). This blocks changes. 
        // Additionally, a copy is stored in this function, which lets the socket
        // continue processing right after the copy and clearing of the previous
        // entities have been handled
        // This function is also run from a loop, which is blocking in terms of 
        // other loops and timers. This means while this function is processing, 
        // no loops will be accessing the entities map. 
        // Thread safety first, kids!
        std::lock_guard<std::mutex> lock(this->wrapper.getEntityGuard());
        updateEntities = wrapper.getUpdateEntities();   
        wrapper.getUpdateEntities().clear();
    }

    wrapper.deleteEntities();

    auto& entities = wrapper.getEntities();
    for (auto& [id, data] : updateEntities) {
        if (entities.find(id) != entities.end()) {
            entities[id].update(data);
        } else entities[id] = data;
    }

    auto now = Types::Clock::now();

    const double delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
    last = now;
    double cDelta = delta;

    while (cDelta > 0) {
        if (this->isAlive() && this->isMoving()) {

            nlohmann::json& entity = this->getRawJson();
            if (entity.find("ref_speed") == entity.end() ||
                (entity.find("ref_speed") != entity.end() && entity["ref_speed"] != entity["speed"])) {
                entity["ref_speed"] = entity["speed"];
                entity["from_x"] = entity["x"];
                entity["from_y"] = entity["y"];
                std::pair<int, int> vxy = MovementMath::calculateVelocity(entity);
                entity["vx"] = vxy.first;
                entity["vy"] = vxy.second;
                entity["engaged_move"] = entity["move_num"];
            }
            MovementMath::moveEntity(entity, cDelta);
            MovementMath::stopLogic(entity);
        }

        for (auto& [id, entity] : wrapper.getEntities()) {

            if (entity.find("speed") == entity.end() && entity["type"] == "monster") {
                std::string type = entity["mtype"];
                entity["speed"] = this->info->G->getData()["monsters"][type]["speed"].get<double>();
            }
            if (!getOrElse(entity, "rip", false) && !getOrElse(entity, "dead", false) &&
                getOrElse(entity, "moving", false)) {
                if (entity.value("move_num", 0l) != entity.value("engaged_move", 0l) ||
                    (entity.find("ref_speed") != entity.end() && entity["ref_speed"] != entity["speed"])) {
                    entity["ref_speed"] = entity["speed"];
                    entity["from_x"] = entity["x"];
                    entity["from_y"] = entity["y"];
                    std::pair<int, int> vxy = MovementMath::calculateVelocity(entity);
                    entity["vx"] = vxy.first;
                    entity["vy"] = vxy.second;

                    entity["engaged_move"] = entity["move_num"];
                }

                MovementMath::moveEntity(entity, cDelta);
                MovementMath::stopLogic(entity); // Processes whether we're done moving or not.
            }
        }

        cDelta -= 50;
    }
}


void BotSkeleton::start() {
    wrapper.connect();
};
void BotSkeleton::stop() {
    wrapper.close();
};