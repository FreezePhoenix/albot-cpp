#ifndef ALBOT_PARSINGUTILS_HPP_
#define ALBOT_PARSINGUTILS_HPP_

// Thanks to LunarWatcher for this file.

#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#include <nlohmann/json.hpp>
#endif

template <typename T, typename K>
inline T getOrElse(const nlohmann::json& n, K key,  T defaultValue) {
	if (!n.is_object()) {
		return defaultValue;
	}
	if (n.find(key) == n.end()) return defaultValue;
	if (n[key].is_null()) return defaultValue;
	return n[key].template get<T>(); 
}


#endif /* ALBOT_PARSINGUTILS_HPP_ */