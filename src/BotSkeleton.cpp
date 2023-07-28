#include <condition_variable>
#include <iostream>
#include <string>

#include "albot/Utils/Timer.hpp"
#include "albot/BotSkeleton.hpp"
#include "albot/MovementMath.hpp"
#include "albot/Utils/ParsingUtils.hpp"

BotSkeleton::BotSkeleton(const CharacterGameInfo& id): Bot(id), loop(), wrapper(std::to_string(info.character->id), this->info.server->url, *this), uvThread([this]() {
			while (running) {
				loop_running.wait(false); // wait until it has changed FROM false to true.
				if(!running) {
					break;
				}
				loop.run();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			this->disconnect();
		}) {
    loop.setInterval([this]() {
        this->processInternals();
    }, 1000.0 / 60.0); 
}

const Types::TimePoint epoch;


bool within_xy_range(double o_x, double o_y, const nlohmann::json& entity) {
	double x = entity["x"].get<double>();
	double y = entity["y"].get<double>();
	if (o_x - 700 < x && x < o_x + 700 && o_y - 500 < y && y < o_y + 500) {
		return true;
	}
	return false;
}

void BotSkeleton::processInternals() {
    if (last == epoch) last = Types::Clock::now();

		std::map<std::string, nlohmann::json> updateEntities;

   		nlohmann::json updatePlayer;
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
			updateEntities = std::move(wrapper.getUpdateEntities());
			updatePlayer = std::move(wrapper.getUpdateCharacter());
			wrapper.getUpdateEntities().clear();
			wrapper.getUpdateCharacter().clear();
		}

		// Shouldn't need to delete entities.
		// wrapper->deleteEntities();

		auto& entities = wrapper.getEntities();
		for (auto& [id, data] : updateEntities) {
			if (entities.find(id) != entities.end()) {
				entities[id].update(data);
			} else entities[id] = data;
		}
		if (!updatePlayer.is_null()) {
			wrapper.getCharacter().update(updatePlayer);
		}

		auto now = Types::Clock::now();

		const double delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
		last = now;
		double cDelta = delta;

		while (cDelta > 0) {
			if (this->isAlive() && this->isMoving()) {
				nlohmann::json& entity = this->getCharacter();
				if (!entity.contains("ref_speed") || entity["ref_speed"] != entity["speed"]) {
					entity["ref_speed"] = entity["speed"];
					entity["from_x"] = entity["x"];
					entity["from_y"] = entity["y"];
					std::pair<double, double> vxy = MovementMath::calculateVelocity(entity);
					entity["vx"] = vxy.first;
					entity["vy"] = vxy.second;
					entity["engaged_move"] = entity["move_num"];
				}
				MovementMath::moveEntity(entity, cDelta);
				MovementMath::stopLogic(entity);
			}
			double o_x = getX();
			double o_y = getY();
			auto& entities = wrapper.getEntities();
			const std::string player_map = getMap();
			for (auto it = entities.begin(); it != entities.end();) {
				auto& [id, entity] = *it;
				bool REMOVE = false;
				if (entity.is_null()) {
					REMOVE = true;
				} else if (entity.value("dead", false)) {
					REMOVE = true;
				} else if (!within_xy_range(o_x, o_y, entity)) {
					REMOVE = true;
				} else if (entity.value("rip", false)) {
					REMOVE = true;
				} else {
					auto it = entity.find("map");
					if (it != entity.end()) {
						if (it->get<std::string>() != player_map) {
							REMOVE = true;
						}
					} else {
						REMOVE = true;
					}
				}
				if (REMOVE) {
					it = entities.erase(it);
				} else {
					++it;
				}
			}
			for (auto& [id, entity] : wrapper.getEntities()) {

				if (entity.find("speed") == entity.end() && entity["type"] == "monster") {
					std::string type = entity["mtype"];
					entity["speed"] = this->info.G->getData()["monsters"][type]["speed"].get<double>();
				}
				if (!entity.value("rip", false) && !entity.value("dead", false) && entity.value("moving", false)) {
					if (entity.value("move_num", 0l) != entity.value("engaged_move", 0l) ||
						(entity.find("ref_speed") != entity.end() && entity["ref_speed"] != entity["speed"])) {
						entity["ref_speed"] = entity["speed"];
						entity["from_x"] = entity["x"];
						entity["from_y"] = entity["y"];
						std::pair<double, double> vxy = MovementMath::calculateVelocity(entity);
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


void BotSkeleton::onDisconnect(std::string reason) {
	mLogger->info("Disconnected: {}", reason);
	if(reason == "Abnormal closure") {
		mLogger->info("Attempting reconnection.");
		loop_running = false;
		std::thread([this]() {
			mLogger->info("{}", (size_t) this->wrapper.getReadyState());
			mLogger->info("Disconnecting.");
			this->disconnect();
			mLogger->info("Reconnecting.");
			this->connect();
			mLogger->info("Connected.");
		}).detach();
	} else {
		mLogger->info("Stopping.");
		this->stop();
	}
}
void BotSkeleton::onConnect() {
	loop_running = true;
	loop_running.notify_all();
}

void BotSkeleton::connect() {
    wrapper.connect();
};

void BotSkeleton::disconnect() {
	wrapper.close();
}
void BotSkeleton::stop() {
	running = false;
};

nlohmann::json& BotSkeleton::getUpdateCharacter() {
	return wrapper.getUpdateCharacter();
}

nlohmann::json& BotSkeleton::getCharacter() {
	return wrapper.getCharacter();
}