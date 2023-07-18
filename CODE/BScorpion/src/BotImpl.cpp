#include <iostream>
#include <string>
#include <mutex>

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

#include "albot/BotSkeleton.hpp"

#ifndef CHARACTER_NAME
#define CHARACTER_NAME -1
#endif

#ifndef CHARACTER_CLASS
#define CHARACTER_CLASS -1
#endif

template<typename RESULT, typename ARGUMENTS>
RESULT invoke_service(const std::string& name, ARGUMENTS* arguments) {
	return ALBot::invoke_service<ARGUMENTS, RESULT>(name, arguments);
}

LightSocket buildLightSocket(SocketWrapper& wrapper) {
	return {
		std::bind_front(&SocketWrapper::registerEventCallback, std::ref(wrapper)),
		std::bind_front(&SocketWrapper::emit, std::ref(wrapper)),
		std::bind_front(&SocketWrapper::getEntities, std::ref(wrapper)),
		std::bind_front(&SocketWrapper::getCharacter, std::ref(wrapper))
	};
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

const ArmorManager::ArmorSet LUCK_SET = ArmorManager::ArmorSet()
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

const ArmorManager::ArmorSet DPS_SET = ArmorManager::ArmorSet()
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

const ArmorManager::ArmorSet GOLD_SET = ArmorManager::ArmorSet()
.add_item("gloves", "handofmidas")
.add_item("ring2", "goldring", 0).build();

class BotImpl : public BotSkeleton {
public:
	std::string STATE = "farm";
	std::optional<EVENT_ENTRY> curEvent = std::nullopt;
	LightLoop lightLoop;
	LightSocket lightSocket;
	Targeter targeter;
	SkillHelper skill_helper;
	void use_hp() {
		if (skill_helper.can_use("potion")) {
			const auto& items = getCharacter()["items"];
			for (size_t i = 0; i < getCharacter()["isize"].get<size_t>(); i++) {
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
			const auto& items = getCharacter()["items"];
			for (size_t i = 0; i < getCharacter()["isize"].get<size_t>(); i++) {
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
			return targeter.get_priority_target(getX(), getY(), wrapper.getEntities(), false, true, true);
		} else {
			return targeter.get_priority_target(getX(), getY(), wrapper.getEntities(), true, true, false);
		}
	};

	auto find_viable_target_ignore_fire() {
		if (curEvent.has_value()) {
			return targeter.get_priority_target(getX(), getY(), wrapper.getEntities(), false, false, true);
		} else {
			return targeter.get_priority_target(getX(), getY(), wrapper.getEntities(), true, true, false);
		}
	};

	void move(double x, double y) {
		auto& data = getCharacter();
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
		if constexpr (CHARACTER_CLASS == ClassEnum::PRIEST) {
			if (skill_helper.can_use("attack")) {
				for (const std::string& party_member : PARTY) {
					if (party_member == name) {
						const auto& member = getCharacter();
						if (Functions::needs_hp(member)) {
							const auto& target = member["target"];
							skill_helper.mark_used("attack");
							wrapper.emit("heal", { { "id", party_member } });
							wrapper.emit("target", { {"id", target } });
							break;
						}
					} else {
						auto it = entities.find(party_member);
						if (it != entities.end()) {
							const auto& member = it->second;
							if (Functions::needs_hp(member) && distance(getCharacter(), member) < getRange()) {
								const auto& target = getCharacter()["target"];
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
					LUCK_SET.attempt_equip(lightSocket);
				}
			}

			if (distance(getCharacter(), monster_target) < getRange()) {
				if (CHARACTER_CLASS == ClassEnum::PRIEST) {
					if (skill_helper.can_use("darkblessing") && getCharacter()["s"].contains("warcry")) {
						skill_helper.mark_used("darkblessing");
						wrapper.emit("skill", { {"name", "darkblessing"} });
					}
					skill_helper.attempt_targeted("curse", monster_target_id);
					if (monster_target["hp"].get<long>() > 20000) {
						skill_helper.attempt_attack(monster_target_id);
					}
				} else if (CHARACTER_CLASS == ClassEnum::WARRIOR) {
					if (skill_helper.can_use("warcry") && !(getCharacter()["s"].contains("warcry"))) {
						skill_helper.mark_used("warcry");
						wrapper.emit("skill", { {"name", "warcry"} });
					}
					skill_helper.attempt_attack(monster_target_id);
				}
			}
			if constexpr (CHARACTER_CLASS == ClassEnum::WARRIOR) {
				if(!isMoving()) {
					if (distance(getCharacter(), monster_target) > 0.5 * getRange()) {
						move(monster_target["x"].get<double>(), monster_target["y"].get<double>());
					}
				}
			}
		} else {
			if constexpr (CHARACTER_CLASS == ClassEnum::PRIEST) {
				auto to_kite = find_viable_target_ignore_fire();
				if (!to_kite.has_value()) {
					if(distance(std::pair{getX(), getY()}, {-420, -1100}) > 10 && !isMoving()) {
						move(-420, -1100);
					}
				}
			}
			if constexpr (CHARACTER_CLASS == ClassEnum::WARRIOR) {
				if (entities.contains("Geoffriel") && !entities.at("Geoffriel")["rip"].get<bool>()) {
					if(distance(std::pair{getX(), getY()}, {-398, -1261.5}) > 10 && !isMoving()) {
						move(-398, -1261.5);
					}
				} else {
					if(distance(std::pair{getX(), getY()}, {-420, -1410}) > 10 && !isMoving()) {
						move(-420, -1410);
					}
				}
			}

		}
	}
	BotImpl(const CharacterGameInfo& id) : BotSkeleton(id), lightLoop(buildLightLoop(loop)), lightSocket(buildLightSocket(wrapper)),  targeter(info.character->name, { "bscorpion" }, PARTY, false, false, CHARACTER_CLASS == ClassEnum::PRIEST), skill_helper(lightLoop, lightSocket) {

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
			if (distance(getCharacter(), chest) < 200.0) {
				GOLD_SET.attempt_equip(lightSocket);
				const std::string id = chest["id"].get<std::string>();
				wrapper.emit("open_chest", {
					{"id", id}
				});
				DPS_SET.attempt_equip(lightSocket);
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
		}, 1000.0 / 60.0);
		loop.setInterval([&]() {
			if constexpr (CHARACTER_CLASS == ClassEnum::WARRIOR) {
				if (skill_helper.can_use("potion")) {
					if (Functions::needs_hp(getCharacter()) && !wrapper.getEntities().contains("Geoffriel")) {
						use_hp();
					} else if (Functions::needs_mp(getCharacter())) {
						use_mp();
					}
				}
			}
			if constexpr (CHARACTER_CLASS == ClassEnum::PRIEST) {
				if (Functions::needs_mp(getCharacter()) && skill_helper.can_use("potion")) {
					use_mp();
				}
			}
		}, 200.0);
		if constexpr (CHARACTER_CLASS == ClassEnum::PRIEST) {
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
};


std::unique_ptr<BotImpl> BotInstance;

void ipc_handler(Message message) {
	if (message.command == "code_message") {
		BotInstance->onCm(message.requester, *((nlohmann::json*)message.arguments));
	} else if (message.command == "code_message_fail") {

	}
}

void cleanup() {
	BotInstance.reset(nullptr);
}

extern "C" std::thread& init(CharacterGameInfo & info) {
	info.child_handler = ipc_handler;
	info.destructor = cleanup;
	
	BotInstance.reset(new BotImpl(info));

	BotInstance->connect();
	return BotInstance->uvThread;
}