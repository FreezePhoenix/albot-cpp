#include "Targeter.hpp"
#include "Functions.hpp"

Targeter::Targeter(const LightSocket& wrapper, const std::string& character_name, const std::vector<std::string>& monster_targets, std::vector<std::string> safe, bool solo, bool require_los, bool tag_targets): socket(wrapper), character_name(character_name), safe(safe) {
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
	auto status_it = entity.find("s");
	if(status_it == entity.end()) {
		return false;
	}
	const auto& status = *status_it;
	
	auto burn_status_it = status.find("burned");
	if(burn_status_it == status.end()) {
		return false;
	}
	const auto& burn_status = *burn_status_it;

	auto intensity_it = burn_status.find("intensity");
	if(intensity_it == burn_status.end()) {
		return false;
	}
	auto ms_it = burn_status.find("ms");
	if(ms_it == burn_status.end()) {
		return false;
	}
	
	double burn_intensity = *intensity_it;
	double ms_remaining = *ms_it;
	double damage_per_tick = burn_intensity / 5;
	double ticks_remaining = std::floor(ms_remaining / 240);
	double damage_predicted = damage_per_tick * ticks_remaining;
	return damage_predicted > entity["hp"];
}

bool Targeter::should_target_entity(const nlohmann::json& entity, bool event) {
	if(!entity.contains("type")) {
		return false;
	}
	if (entity["type"].get<std::string>() == "monster") {
		const std::string& mtype = entity["mtype"];
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

std::optional<std::reference_wrapper<const nlohmann::json>> Targeter::get_priority_target(bool any, bool ignore_fire, bool event) {
	const auto& entities = socket.entities();
	const auto& character = socket.character();
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
								Functions::distance_squared(character, entity)
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
		return std::min_element(potentialTargets.begin(), potentialTargets.end(), [](const SORT_ENTRY& first, const SORT_ENTRY& second) {
			return first.first < second.first;
		})->second;
	}
}