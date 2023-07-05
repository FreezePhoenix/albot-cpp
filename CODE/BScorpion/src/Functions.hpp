#ifndef BOTIMPL_FUNCTIONS_HPP_
#define BOTIMPL_FUNCTIONS_HPP_

#include <nlohmann/json.hpp>
namespace Functions {
	bool needs_mp(const nlohmann::json& entity);
	bool needs_hp(const nlohmann::json& entity);
}

#endif