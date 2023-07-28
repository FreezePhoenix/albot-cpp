#ifndef BOTIMPL_SKILLHELPER_HPP_
#define BOTIMPL_SKILLHELPER_HPP_

#include "LightSocket.hpp"
#include "LightLoop.hpp"
#include <set>

struct SkillHelper {
private:
	std::set<std::string> unusable;
	const LightLoop& loop;
	const LightSocket& socket;
	std::chrono::duration<uint64_t, std::milli> last_ping;
	std::vector<size_t> pings;
	size_t ping_sum;
	bool can_use_internal(const std::string& skill) const;
	void mark_used_internal(const std::string& skill);
	void clear_cooldown(const std::string& skill);
public:
	mutable std::mutex skill_guard;
	size_t ping = 0;
	SkillHelper(const LightLoop& lightLoop, const LightSocket& lightSocket);
	bool can_use(const std::string& skill) const;
	void mark_used(const std::string& skill);
	void set_cooldown(std::string skill, size_t millis);
	void attempt_attack(const nlohmann::json& entity);
	void attempt_heal(const nlohmann::json& entity);
	void attempt_targeted(const std::string& skill, const nlohmann::json& entity);
	void attempt(const std::string& skill);
	void attempt_use_hp_potion();
	void attempt_use_mp_potion();
};

#endif