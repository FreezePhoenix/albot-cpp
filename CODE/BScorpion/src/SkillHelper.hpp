#ifndef BOTIMPL_SKILLHELPER_HPP_
#define BOTIMPL_SKILLHELPER_HPP_

#include "LightSocket.hpp"
#include "LightLoop.hpp"

struct SkillHelper {
private:
	std::map<std::string, bool> can_use_map;
	const LightLoop& loop;
	const LightSocket& socket;
	std::chrono::duration<uint64_t, std::milli> last_ping;
	std::vector<size_t> pings;
	size_t ping_sum;
	bool can_use_internal(std::string skill);
	void mark_used_internal(std::string skill);
public:
	mutable std::mutex skill_guard;
	size_t ping = 0;
	SkillHelper(const LightLoop& lightLoop, const LightSocket& lightSocket);
	bool can_use(std::string skill);
	void set_cooldown(std::string skill, size_t millis);
	void mark_used(std::string skill);
	void attempt_attack(std::string id);
	void attempt_heal(std::string id);
	void attempt_targeted(std::string skill, std::string id);
};

#endif