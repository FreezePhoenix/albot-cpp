#include "SkillHelper.hpp"

SkillHelper::SkillHelper(const LightLoop& lightLoop, const LightSocket& lightSocket) : loop(lightLoop), socket(lightSocket) {
	socket.on("game_response", [this](const nlohmann::json& event) {
		auto response_it = event.find("response");
		if(response_it == event.end()) {
			return;
		}
		if(*response_it == "cooldown") {
			auto failed_it = event.find("failed");
			if(failed_it == event.end()) {
				return;
			}
			if(*failed_it == true) {
				auto skill_it = event.find("skill");
				if(skill_it == event.end()) {
					return;
				}
				auto ms_it = event.find("ms");
				if(ms_it == event.end()) {
					return;
				}
				std::string skill = skill_it->get<std::string>();
				size_t ms = ms_it->get<size_t>();
				if(skill == "attack" || skill == "heal") {
					set_cooldown("attack", ms);
				}
				if(skill == "curse") {
					set_cooldown("curse", ms);
				}
			}
		}
	});
	socket.on("disappear", [this](const nlohmann::json& event) {
		auto reason_it = event.find("reason");
		if(reason_it == event.end()) {
			return;
		}
		if(*reason_it == "not_there") {
			auto place_it = event.find("place");
			if(place_it == event.end()) {
				return;
			}
			std::lock_guard<std::mutex> guard(skill_guard);
			clear_cooldown(place_it->get<std::string>());
		}
	});
	socket.on("ping_ack", [this](const nlohmann::json& event) {
		std::lock_guard<std::mutex> guard(skill_guard);
		if (pings.size() > 63) {
			ping_sum -= pings.front();
			pings.erase(pings.begin());
		}
		const size_t& ping_time = pings.emplace_back((loop.now() - last_ping).count());
		ping_sum += ping_time;
		ping = ping_sum / pings.size();
	});
	socket.on("skill_timeout", [this](const nlohmann::json& event) {
		auto name_it = event.find("name");
		if (name_it == event.end()) {
			return;
		}
		auto ms_it = event.find("ms");
		if (ms_it == event.end()) {
			return;
		}
		std::string name = name_it->get<std::string>();
		size_t ms = ms_it->get<size_t>();
		set_cooldown(name, ms);
	});
	socket.on("eval", [this](const nlohmann::json& event) {
		auto code_it = event.find("code");
		if (code_it == event.end()) {
			return;
		}
		if (code_it->get<std::string>().starts_with("pot_timeout")) {
			set_cooldown("potion", 2000);
		}
	});
	loop.setInterval([this]() {
		std::lock_guard<std::mutex> guard(skill_guard);
		last_ping = loop.now();
		socket.emit("ping_trig", { "id", "pings" });
	}, 4000);
}

bool SkillHelper::can_use(const std::string& name) const {
	std::lock_guard<std::mutex> guard(skill_guard);
	return !unusable.contains(name);
}

void SkillHelper::mark_used(const std::string& name) {
	std::lock_guard<std::mutex> guard(skill_guard);
	unusable.emplace(name);
}

void SkillHelper::set_cooldown(std::string name, size_t millis) {
	{
		std::lock_guard<std::mutex> guard(skill_guard);
		if(millis <= ping) {
			clear_cooldown(name);
			return;
		} else {
			millis -= ping;
		}
	}
	loop.exec([this, TIMEOUT = [this, name = std::move(name)]() {
		std::lock_guard<std::mutex> guard(skill_guard);
		clear_cooldown(name);
	}, millis]() mutable {
		loop.setTimeout(std::move(TIMEOUT), millis);
	});
}

inline bool SkillHelper::can_use_internal(const std::string& name) const {
	return !unusable.contains(name);
}

inline void SkillHelper::mark_used_internal(const std::string& name) {
	unusable.emplace(name);
}

inline void SkillHelper::clear_cooldown(const std::string& name) {
	unusable.erase(name);
}

void SkillHelper::attempt_attack(const nlohmann::json& entity) {
	auto id_it = entity.find("id");
	if(id_it == entity.end()) {
		return;
	} else {
		std::lock_guard<std::mutex> guard(skill_guard);
		if (can_use_internal("attack")) {
			mark_used_internal("attack");
			socket.emit("attack", {
				{"id", *id_it }
			});
		}
	}
}

void SkillHelper::attempt_heal(const nlohmann::json& entity) {
	auto id_it = entity.find("id");
	if(id_it == entity.end()) {
		return;
	} else {
		std::lock_guard<std::mutex> guard(skill_guard);
		if (can_use_internal("attack")) {
			mark_used_internal("attack");
			socket.emit("heal", {
				{"id", *id_it }
			});
		}
	}
}

void SkillHelper::attempt_targeted(const std::string& skill, const nlohmann::json& entity) {
	auto id_it = entity.find("id");
	if(id_it != entity.end()) {
		std::lock_guard<std::mutex> guard(skill_guard);
		if (can_use_internal(skill)) {
			mark_used_internal(skill);
			socket.emit("skill", {
				{ "name", skill },
				{ "id", *id_it }
			});
		}
	}
}

void SkillHelper::attempt(const std::string& skill) {
	std::lock_guard<std::mutex> guard(skill_guard);
	if (can_use_internal(skill)) {
		mark_used_internal(skill);
		socket.emit("skill", {
			{ "name", skill }
		});
	}
}

void SkillHelper::attempt_use_mp_potion() {
	const auto& character = socket.character();
	std::lock_guard<std::mutex> guard(skill_guard);
	if (can_use_internal("potion")) {
			const auto& items = character["items"];
			for (size_t i = 0; i < character["isize"].get<size_t>(); i++) {
				const auto& item = items[i];
				if(item.contains("name")) {
					std::string name = item["name"];
					if (name.starts_with("mpot")) {
							mark_used_internal("potion");
							socket.emit("equip", { {"num", i} });
							break;
					}
				}
			}
	}
}

void SkillHelper::attempt_use_hp_potion() {
	const auto& character = socket.character();
	std::lock_guard<std::mutex> guard(skill_guard);
	if (can_use_internal("potion")) {
			const auto& items = character["items"];
			for (size_t i = 0; i < character["isize"].get<size_t>(); i++) {
				const auto& item = items[i];
				if(item.contains("name")) {
					std::string name = item["name"];
					if (name.starts_with("hpot")) {
							mark_used_internal("potion");
							socket.emit("equip", { {"num", i} });
							break;
					}
				}
			}
	}
}