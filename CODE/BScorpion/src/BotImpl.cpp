#include <iostream>
#include <string>
#include <mutex>

#include "albot/SocketWrapper.hpp"
#include "albot/Utils/LoopHelper.hpp"
#include "albot/Utils/Timer.hpp"
#include "albot/MovementMath.hpp"

#include "albot/albot-cpp.hpp"
#include "Targeter.hpp"
#include "Functions.hpp"
#include "ArmorManager.hpp"
#include "SkillHelper.hpp"
#include "LightSocket.hpp"
#include "LightLoop.hpp"
#include "../../SERVICES/Pathfinding/include/Pathfinding/Service.hpp"
#include <mutex>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

#ifndef CHARACTER_NAME
#define CHARACTER_NAME -1
#endif

#ifndef CHARACTER_CLASS
#define CHARACTER_CLASS -1
#endif


// extern std::map<std::string, ServiceInfo<void*, void>*> ALBot::SERVICE_HANDLERS;
const Types::TimePoint epoch;

template<typename RESULT, typename ARGUMENTS>
RESULT invoke_service(const std::string& name, ARGUMENTS* arguments) {
	return ALBot::invoke_service<ARGUMENTS, RESULT>(name, arguments);
}

LightSocket buildLightSocket(SocketWrapper& wrapper) {
	return LightSocket(
		std::bind_front(&SocketWrapper::registerEventCallback, std::ref(wrapper)),
		std::bind_front(&SocketWrapper::emit, std::ref(wrapper)),
		std::bind_front(&SocketWrapper::getEntities, std::ref(wrapper))
	);
}

LightLoop buildLightLoop(LoopHelper& loop) {
	return LightLoop(
		std::bind_front(&LoopHelper::setInterval, std::ref(loop)),
		std::bind_front(&LoopHelper::setTimeout, std::ref(loop)),
		std::bind_front(&LoopHelper::exec, std::ref(loop)),
		std::bind_front(&LoopHelper::now, std::ref(loop))
	);
}


const std::vector<std::string> PARTY = { "Rael", "Raelina", "Geoffriel" };

struct EVENT_ENTRY {
	double x;
	double y;
	std::string NAME;
	std::string MAP;
};

inline ArmorManager::ArmorSet LUCK_SET = ArmorManager::ArmorSet()
.add_item("helmet", "wcap", 7)
.add_item("earring2", "dexearringx", 0)
.add_item("amulet", "spookyamulet")
.add_item("mainhand", "lmace", 7)
.add_item("chest", "wattire", 6)
.add_item("offhand", "mshield", 7)
.add_item("cape", "ecape", 8)
.add_item("pants", "wbreeches", 8)
.add_item("ring2", "ringofluck", 0)
.add_item("orb", "rabbitsfoot", 2)
.add_item("gloves", "wgloves").build();

inline ArmorManager::ArmorSet DPS_SET = ArmorManager::ArmorSet()
.add_item("helmet", "gphelmet", 6)
.add_item("earring2", "cearring", 4)
.add_item("amulet", "intamulet", 5)
.add_item("mainhand", "firestaff", 8)
.add_item("chest", "cdragon")
.add_item("offhand", "wbookhs", 2)
.add_item("cape", "angelwings", 8)
.add_item("pants", "starkillers", 7)
.add_item("ring2", "cring", 4)
.add_item("orb", "jacko", 4)
.add_item("gloves", "supermittens", 7).build();

inline ArmorManager::ArmorSet GOLD_SET = ArmorManager::ArmorSet()
.add_item("gloves", "handofmidas")
.add_item("ring2", "goldring", 0).build();

class BotImpl : public Bot {
	std::mutex m;
	std::condition_variable cv;
public:
	std::promise<std::thread*> loop_thread = std::promise<std::thread*>();
	Types::TimePoint last;
	bool running = false;
	std::thread uvThread;
	std::string STATE = "farm";
	std::optional<EVENT_ENTRY> curEvent = std::nullopt;
	std::shared_ptr<spdlog::logger> mLogger;
	LoopHelper loop;
	SocketWrapper wrapper;
	LightLoop lightLoop;
	LightSocket lightSocket;
	Targeter targeter;
	SkillHelper skill_helper;
	void use_hp() {
		if (skill_helper.can_use("potion")) {
			const auto& items = data["items"];
			for (size_t i = 0; i < data["isize"].get<size_t>(); i++) {
				const auto& item = items[i];
				if (item.is_object()) {
					if (item["name"].is_string()) {
						std::string name = item["name"].get<std::string>();
						if (name.starts_with("hpot")) {
							skill_helper.mark_used("potion");
							wrapper.emit("equip", { {"num", i} });
							break;
						}
					}
				}
			}
		}
	}
	void use_mp() {
		if (skill_helper.can_use("potion")) {
			const auto& items = data["items"];
			for (size_t i = 0; i < data["isize"].get<size_t>(); i++) {
				const auto& item = items[i];
				if (item.is_object()) {
					if (item["name"].is_string()) {
						std::string name = item["name"].get<std::string>();
						if (name.starts_with("mpot")) {
							skill_helper.mark_used("potion");
							wrapper.emit("equip", { {"num", i} });
							break;
						}
					}
				}
			}
		}
	}

	auto get_kite_point(double origin_x, double origin_y, double target_x, double target_y, double range, bool clockwise) {
		double mod = 1;
		if (!clockwise) {
			mod = -1;
		}

		double opp = target_y - origin_y;
		double adj = target_x - origin_x;
		double hypot = std::hypot(opp, adj);
		double THETA = NAN;
		double YDIF = NAN;
		if (adj > 0) {
			THETA = std::asin(opp / hypot) + mod * ((8.0 * std::numbers::pi) / 12.0);
			YDIF = range * std::sin(THETA);
		} else {
			THETA = std::asin(opp / hypot) + mod * ((4.0 * std::numbers::pi) / 12.0);
			YDIF = -range * std::sin(THETA);
		}
		double XDIF = range * std::cos(THETA);
		return std::make_pair(origin_x - XDIF, origin_y - YDIF);
	};
	auto distance(std::pair<double, double> A, std::pair<double, double> B) {
		return std::hypot(A.first - B.first, A.second - B.second);
	};
	auto distance(const nlohmann::json& A, const nlohmann::json& B) {
		return std::hypot(A["x"].get<double>() - B["x"].get<double>(), A["y"].get<double>() - B["y"].get<double>());
	};
	auto determine_clockwise(double origin_x, double origin_y, double target_x, double target_y, double range) {
		auto CW = get_kite_point(origin_x, origin_y, target_x, target_y, range, true);
		auto ACW = get_kite_point(origin_x, origin_y, target_x, target_y, range, false);
		auto CHAR_LOC = std::make_pair(getX(), getY());
		return distance(CHAR_LOC, CW) < distance(CHAR_LOC, ACW);
	};

	auto find_viable_target() {
		if (curEvent.has_value()) {
			return targeter.get_priority_target(data["x"].get<double>(), data["y"].get<double>(), wrapper.getEntities(), false, true, true);
		} else {
			return targeter.get_priority_target(data["x"].get<double>(), data["y"].get<double>(), wrapper.getEntities(), true, true, false);
		}
	};

	auto find_viable_target_ignore_fire() {
		if (curEvent.has_value()) {
			return targeter.get_priority_target(data["x"].get<double>(), data["y"].get<double>(), wrapper.getEntities(), false, false, true);
		} else {
			return targeter.get_priority_target(data["x"].get<double>(), data["y"].get<double>(), wrapper.getEntities(), true, true, false);
		}
	};

	void move(double x, double y) {
		data["from_x"] = data["x"];
		data["from_y"] = data["y"];
		data["going_x"] = x;
		data["going_y"] = y;
		data["moving"] = true;

		auto vxy = MovementMath::calculateVelocity(data);
		data["vx"] = vxy.first;
		data["vy"] = vxy.second;
		wrapper.emit("move", {
			{"x", getX()},
			{"y", getY()},
			{"going_x", x},
			{"going_y", y},
			{"m", getMapId() }
		});
	}
	void state_controller() {
		if (curEvent.has_value()) {
			STATE = "event";
		} else {
			STATE = "farm";
		}
	}
	void farm(std::optional<EVENT_ENTRY> event = std::nullopt) {
		const auto& entities = wrapper.getEntities();
		if (CHARACTER_CLASS == ClassEnum::PRIEST) {
			if (skill_helper.can_use("attack")) {
				for (const std::string& party_member : PARTY) {
					if (party_member == name) {
						const auto& member = data;
						if (Functions::needs_hp(member)) {
							const auto& target = data["target"];
							skill_helper.mark_used("attack");
							wrapper.emit("heal", { { "id", party_member } });
							wrapper.emit("target", { {"id", target } });
							break;
						}
					} else {
						auto it = entities.find(party_member);
						if (it != entities.end()) {
							const auto& member = it->second;
							if (Functions::needs_hp(member) && distance(data, member) < getRange()) {
								const auto& target = data["target"];
								skill_helper.mark_used("attack");
								wrapper.emit("heal", { { "id", party_member } });
								wrapper.emit("target", { {"id", target } });
								break;
							}
						}
					}
				}
			}
		}
		auto attack_target = find_viable_target();
		if (attack_target.has_value()) {
			const nlohmann::json& monster_target = attack_target.value();
			const std::string& monster_target_id = monster_target["id"].get<std::string>();
			if (CHARACTER_CLASS == ClassEnum::PRIEST) {

				if (!monster_target.contains("target") || monster_target["target"].is_null()) {
					skill_helper.attempt_targeted("zapperzap", monster_target_id);
				}
				if (monster_target["hp"].get<long>() / (double)monster_target["max_hp"].get<long>() < 0.2 && monster_target["mtype"].get<std::string>() != "bgoo") {
					LUCK_SET.ensure_equipped(data, lightSocket);
				}
			}

			if (distance(data, monster_target) < getRange()) {
				if (CHARACTER_CLASS == ClassEnum::PRIEST) {
					if (skill_helper.can_use("darkblessing") && data["s"].contains("warcry")) {
						skill_helper.mark_used("darkblessing");
						wrapper.emit("skill", { {"name", "darkblessing"} });
					}
					skill_helper.attempt_targeted("curse", monster_target_id);
					if (monster_target["hp"].get<long>() > 20000) {
						skill_helper.attempt_attack(monster_target_id);
					}
				} else if (CHARACTER_CLASS == ClassEnum::WARRIOR) {
					if (skill_helper.can_use("warcry") && !(data["s"].contains("warcry"))) {
						skill_helper.mark_used("warcry");
						wrapper.emit("skill", { {"name", "warcry"} });
					}
					skill_helper.attempt_attack(monster_target_id);
				}
			}
			if (CHARACTER_CLASS == ClassEnum::WARRIOR) {
				if (distance(data, monster_target) > 0.5 * getRange()) {
					move(monster_target["x"].get<double>(), monster_target["y"].get<double>());
				}
			}
		} else {
			if (CHARACTER_CLASS == ClassEnum::PRIEST) {
				auto to_kite = find_viable_target_ignore_fire();
				if (!to_kite.has_value()) {
					move(-420, -1100);
				}
			}
			if (CHARACTER_CLASS == ClassEnum::WARRIOR) {
				if (entities.contains("Geoffriel") && !entities.at("Geoffriel")["rip"].get<bool>()) {
					move(-398, -1261.5);
				} else {
					move(-420, -1410);
				}
			}

		}
	}
	BotImpl(const CharacterGameInfo& id) : Bot(id), loop(), wrapper(std::to_string(info.character->id), this->info.server->url, *this), lightLoop(buildLightLoop(loop)), lightSocket(buildLightSocket(wrapper)),  targeter(info.character->name, { "bscorpion" }, PARTY, false, false, CHARACTER_CLASS == ClassEnum::PRIEST), skill_helper(lightLoop, lightSocket) {
		this->mLogger = spdlog::stdout_color_mt(this->info.character->name + ":BotImpl");
		this->name = info.character->name;
		this->id = info.character->id;
		loop.setInterval([this]() {
			this->processInternals();
		}, 1000.0 / 60.0);
		//===============================
		// BOT CODE AFTER THIS POINT
		//===============================
		wrapper.registerEventCallback("game_response", [this](const nlohmann::json& data) {
			if(data.contains("response")) {
				const std::string res = data["response"].get<std::string>();
				if (res == "defeated_by_a_monster") {
					mLogger->info("Defeated by monsty");
				}
			}
		});
		wrapper.registerEventCallback("drop", [this](const nlohmann::json& chest) {
			if (distance(data, chest) < 200.0) {
				GOLD_SET.ensure_equipped(data, lightSocket);
				const std::string id = chest["id"].get<std::string>();
				wrapper.emit("open_chest", {
					{"id", id}
				});
				DPS_SET.ensure_equipped(data, lightSocket);
			}
		});
		wrapper.registerEventCallback("chest_opened", [this](const nlohmann::json& loot_info) {
			const double goldm = loot_info["goldm"].get<double>();
			const long gold = loot_info["gold"].get<long>();
			const std::string opener = loot_info["opener"].get<std::string>();
			mLogger->info("{} looted a chest with goldm {} giving me {} gold", opener, goldm, gold);
		});
		loop.setInterval([&]() {
			state_controller();
			if (STATE == "farm") {
				farm();
				// curEvent = next_event(curEvent);
			} else if (STATE == "event") {
				farm(curEvent);
				// curEvent = next_event(curEvent);
			}
		}, 200.0);
		loop.setInterval([&]() {
			if (CHARACTER_CLASS == ClassEnum::WARRIOR) {
				if (skill_helper.can_use("potion")) {
					if (Functions::needs_hp(data) && !wrapper.getEntities().contains("Geoffriel")) {
						use_hp();
					} else if (Functions::needs_mp(data)) {
						use_mp();
					}
				}
			}
			if (CHARACTER_CLASS == ClassEnum::PRIEST) {
				if (Functions::needs_mp(data) && skill_helper.can_use("potion")) {
					use_mp();
				}
			}
		}, 200.0);
		if (CHARACTER_CLASS == ClassEnum::PRIEST) {
			loop.setInterval([&]() {
				const double KITING_ORIGIN_X = -450.0;
				const double KITING_ORIGIN_Y = -1240.0;
				const double KITING_RANGE = 121; // Roughly 2/3rds of the character's range
				if (STATE == "farm" && getMap() == "desertland") {
					auto CHAR_LOC = std::make_pair(getX(), getY());
					if (distance(CHAR_LOC, std::make_pair(KITING_ORIGIN_X, KITING_ORIGIN_Y)) < 200.0) {
						auto monster = find_viable_target_ignore_fire();
						if (monster.has_value()) {
							const nlohmann::json& monster_entity = monster.value().get();
							if (monster_entity["mtype"].get<std::string>() != "bscorpion") {
								return;
							}
							double monster_x = monster_entity["x"].get<double>();
							double monster_y = monster_entity["y"].get<double>();
							bool kiting_clockwise = determine_clockwise(KITING_ORIGIN_X, KITING_ORIGIN_Y, monster_x, monster_y, KITING_RANGE);
							auto [x, y] = get_kite_point(KITING_ORIGIN_X, KITING_ORIGIN_Y, monster_x, monster_y, KITING_RANGE, kiting_clockwise);
							move(x, y);
						}
					}
				}
			}, 500.0);
		}
	};
	bool within_xy_range(double o_x, double o_y, const nlohmann::json& entity) {
		double x = entity["x"].get<double>();
		double y = entity["y"].get<double>();
		if (o_x - 700 < x && x < o_x + 700 && o_y - 500 < y && y < o_y + 500) {
			return true;
		}
		return false;
	}
	void processInternals() {
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
			updatePlayer = std::move(updatedData);
			updatedData.clear();
			wrapper.getUpdateEntities().clear();
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
			data.update(updatePlayer);
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
				if (!within_xy_range(o_x, o_y, entity)) {
					entity["dead"] = true;
				}
				bool REMOVE = false;
				if (entity.is_null()) {
					REMOVE = true;
				} else if (entity.value("dead", false)) {
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
	void startUVThread() {
		std::lock_guard lk(m);
		uvThread = std::thread([this]() {
			{
				std::lock_guard lk(m);
				loop.update();
				running = true;
			}
			cv.notify_one();
			while (running) {
				loop.run();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			this->stop();
		});
	}
	void onCm(const std::string& name, const nlohmann::json& data) {
		mLogger->info("{} sent {}", name, data.dump());
	}
	void onConnect() {
		this->log("Connected!?!");
		// loop.exec([this] {
		// 	loop.setTimeout([this] {
		// 		running = false;
		// 	}, 1000);
		// });
		this->startUVThread();
	}
	void start() {
		wrapper.connect();
	}
	void stop() {
		wrapper.close();
	}
	void await_start() {
		{
			std::unique_lock lk(m);
			cv.wait(lk, [this]{return running;});
		}
	}
	void await_stop() {
		std::unique_lock lk(m);
		uvThread.join();
	}
};


std::unique_ptr<BotImpl> BotInstance;

void ipc_handler(Message message) {
	if (message.command == "code_message") {
		BotInstance->onCm(message.requester, *((nlohmann::json*)message.arguments));
	} else if (message.command == "code_message_fail") {
		BotInstance->mLogger->error("Sad face :c");
	}
}

void cleanup() {
	BotInstance->log("DESTRUCTING");
	BotInstance.reset(nullptr);
}

extern "C" std::thread init(CharacterGameInfo & info) {
	info.child_handler = ipc_handler;
	info.destructor = cleanup;
	
	BotInstance.reset(new BotImpl(info));
	BotInstance->log("Class: " + ClassEnum::getClassStringInt(CHARACTER_CLASS));
	BotInstance->log("Logging in... ");
	BotInstance->start();
	return std::thread([] {
		BotInstance->await_start();

		BotInstance->log("Connected");

		BotInstance->await_stop();
	});
}