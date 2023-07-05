#include "SkillHelper.hpp"

SkillHelper::SkillHelper(const LightLoop& lightLoop, const LightSocket& lightSocket) : loop(lightLoop), socket(lightSocket) {
	socket.on("disappear", [this](const nlohmann::json& event) {
		if (event["reason"] == "not_there") {
			if (event.contains("place")) {
				std::lock_guard<std::mutex> guard(skill_guard);
				can_use_map.insert_or_assign(event["place"].get<std::string>(), true);
			}
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
		set_cooldown(name_it->get<std::string>(), ms_it->get<size_t>());
	});
	socket.on("eval", [this](const nlohmann::json& event) {
		auto code_it = event.find("code");
		if (code_it != event.end()) {
			if (code_it->get<std::string>().starts_with("pot_timeout")) {
				set_cooldown("potion", 2000);
			}
		}
	});
	loop.setInterval([this]() {
		last_ping = loop.now();
		socket.emit("ping_trig", { "id", "pings" });
	}, 4000);
}

bool SkillHelper::can_use(std::string name) {
	std::lock_guard<std::mutex> guard(skill_guard);
	auto it = can_use_map.find(name);
	if (it == can_use_map.end()) {
		return true;
	}
	return it->second;
}



void SkillHelper::set_cooldown(std::string name, size_t millis) {
	// When this is received, the result we receive is delayed by the ping, so we subtract half of the ping from the cooldown.
	// Realistically we should be able to subtract the entire ping from the cooldown, but ping is... problematic, because it may decrease.
	loop.setTimeout([this, name]() {
		std::lock_guard<std::mutex> guard(skill_guard);
		can_use_map.insert_or_assign(name, true);
	}, millis);
}

void SkillHelper::mark_used(std::string name) {
	std::lock_guard<std::mutex> guard(skill_guard);
	can_use_map.insert_or_assign(name, false);
}

inline bool SkillHelper::can_use_internal(std::string name) {
	auto it = can_use_map.find(name);
	if (it == can_use_map.end()) {
		return true;
	}
	return it->second;
}

inline void SkillHelper::mark_used_internal(std::string name) {
	can_use_map.insert_or_assign(name, false);
}

void SkillHelper::attempt_attack(std::string id) {
	std::lock_guard<std::mutex> guard(skill_guard);
	if (can_use_internal("attack")) {
		mark_used_internal("attack");
		socket.emit("attack", {
			{"id", id}
		});
	}
}

void SkillHelper::attempt_heal(std::string id) {
	std::lock_guard<std::mutex> guard(skill_guard);
	if (can_use_internal("attack")) {
		mark_used_internal("attack");
		socket.emit("heal", {
			{"id", id}
		});
	}
}

void SkillHelper::attempt_targeted(std::string skill, std::string id) {
	std::lock_guard<std::mutex> guard(skill_guard);
	if (can_use_internal(skill)) {
		mark_used_internal(skill);
		socket.emit("skill", {
			{ "name", skill },
			{ "id", id }
		});
	}
}