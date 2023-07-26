#ifndef BOTIMPL_TARGETER_HPP_
#define BOTIMPL_TARGETER_HPP_


#include <nlohmann/json.hpp>
#include <map>
#include <optional>
#include <memory>
#include "LightSocket.hpp"

class Targeter {
private:
	const LightSocket& socket;
	std::string character_name;
	std::map<std::string, unsigned int> targeting_priorities = {
		{ "pinkgoo", 1 },
		{ "snowman", 1 },
		{ "mrpumpkin", 1 },
		{ "mrgreen", 1 },
		{ "rgoo", 0 },
		{ "bgoo", 1 }
	};
	std::map<std::string, unsigned int> events = {
		{ "mrpumpkin", 1 },
		{ "mrgreen", 1 },
		{ "bgoo", 1 },
		{ "rgoo", 1}
	};
	std::vector<std::string> safe;
	bool solo;
	bool require_los;
	bool tag_targets;
public:
	Targeter(const LightSocket& wrapper, const std::string& character_name, const std::vector<std::string>& monster_targets, std::vector<std::string> safe, bool solo = false, bool require_los = false, bool tag_targets = true);
	bool is_targeting_party(const nlohmann::json& entity);
	static bool will_entity_die_from_fire(const nlohmann::json& entity);
	bool should_target_entity(const nlohmann::json& entity, bool event = false);
	std::optional<std::reference_wrapper<const nlohmann::json>> get_priority_target(bool any = false, bool ignore_fire = false, bool event = false);
};

#endif