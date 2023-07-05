#include "Targeter.hpp"

Targeter::Targeter(const std::string& character_name, const std::vector<std::string>& monster_targets, std::vector<std::string> safe, bool solo, bool require_los, bool tag_targets): character_name(character_name), safe(safe) {
	for (size_t i = 0; i < monster_targets.size(); i++) {
		targeting_priorities.emplace(monster_targets[i], i + 2);
	}
	this->solo = solo;
	this->require_los = require_los;
	this->tag_targets = tag_targets;
}

bool Targeter::is_targeting_party(const nlohmann::json& entity) {
	if (entity.contains("target")) {
		const auto& jtarget = entity["target"];
		if (jtarget.is_string()) {
			const std::string target = jtarget.get<std::string>();
			if (this->solo) {
				return target == character_name;
			}
			return std::find(safe.begin(), safe.end(), target) != safe.end();
		}
	}
	return false;
}

bool Targeter::will_entity_die_from_fire(const nlohmann::json& entity) {
	const auto& status = entity["s"];
	if (status.contains("burned")) {
		const auto& burn_status = status["burned"];
		assert(burn_status.contains("intensity"));
		assert(burn_status.contains("ms"));
		const double burn_intensity = burn_status["intensity"].get<double>();
		const double ms_remaining = burn_status["ms"].get<double>();
		const double damage_per_tick = burn_intensity / 5;
		const double ticks_remaining = std::floor(ms_remaining / 240);
		const double damage_predicted = damage_per_tick * ticks_remaining;
		return damage_predicted > entity["hp"].get<long>();
	}
	return false;
}

bool Targeter::should_target_entity(const nlohmann::json& entity, bool event) {
	assert(entity.contains("type"));
	if (entity["type"].get<std::string>() == "monster") {
		const std::string mtype = entity["mtype"].get<std::string>();
		if (mtype == "grinch") {
			return false;
		}
		if (is_targeting_party(entity)) {
			return true;
		}
		if (targeting_priorities.contains(mtype)) {
			if (event && !events.contains(mtype)) {
				return false;
			}
			if (entity.contains("cooperative")) {
				return true;
			}
			if (tag_targets) {
				
				if (!entity.contains("target")) {
					return true;
				}
				return entity["target"].is_null();
			}
		}
	}
	return false;
}

double distance_squared(double x, double y, const nlohmann::json& entity) {
	double ex = entity["x"].get<double>();
	double ey = entity["y"].get<double>();
	double dx = ex - x;
	double dy = ey - y;
	return dx * dx + dy * dy;
}

std::optional<std::reference_wrapper<const nlohmann::json>> Targeter::get_priority_target(double x, double y, const std::map<std::string, nlohmann::json>& entities, bool any, bool ignore_fire, bool event) {
	if (any) {
		for (const auto& [id, entity] : entities) {
			if (should_target_entity(entity, event)) {
				if (!require_los) {
					// NYI to not have LOS...
					if (ignore_fire || !will_entity_die_from_fire(entity)) {
						return entity;
					}
				}
			}
		}
		return std::nullopt;
	} else {
		using SORT_ENTRY = std::pair<std::tuple<unsigned int, bool, double>, std::reference_wrapper<const nlohmann::json>>;
		std::vector<SORT_ENTRY> potentialTargets = {};
		for (const auto& [id, entity] : entities) {
			if (should_target_entity(entity, event)) {
				if (!require_los) {
					if (ignore_fire || !will_entity_die_from_fire(entity)) {
						potentialTargets.emplace_back(
							std::make_tuple(
								targeting_priorities.at(entity["mtype"].get<std::string>()),
								!is_targeting_party(entity),
								distance_squared(x, y, entity)
							),
							std::cref(entity)
						);
					}
				}
			}
		}
		if (potentialTargets.empty()) {
			return std::nullopt;
		}
		const auto comp = [](const SORT_ENTRY& first, const SORT_ENTRY& second) {
			return first.first < second.first;
		};
		return std::min_element(potentialTargets.begin(), potentialTargets.end(), comp)->second;
	}
}