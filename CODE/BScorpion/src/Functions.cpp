#include "Functions.hpp"

bool Functions::needs_mp(const nlohmann::json& entity) {
	return entity["mp"].get<long>() / (double) entity["max_mp"].get<long>() < 0.75;
}

bool Functions::needs_hp(const nlohmann::json& entity) {
	return entity["hp"].get<long>() / (double) entity["max_hp"].get<long>() < 0.75;
}
