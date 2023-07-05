#ifndef BOTIMPL_ARMORMANAGER_HPP_
#define BOTIMPL_ARMORMANAGER_HPP_

#include "LightSocket.hpp"
#include <nlohmann/json.hpp>
#include <optional>
#include <functional>
#include <concepts>

namespace ArmorManager {
	struct ArmorItemFilter {
		const std::string slot;
		const std::string name;
		const std::optional<unsigned int> level;
		const std::function<bool(unsigned int, unsigned int)> levelMode;
		const std::optional<std::string> property;
		bool test(const nlohmann::json& item) const;
	};

	struct ArmorSet {
	private:
		std::vector<ArmorItemFilter> item_filters;
	public:
		ArmorSet& add_item(std::string&& slot, std::string&& names, std::optional<unsigned int>&& level = std::nullopt, std::function<bool(unsigned int, unsigned int)>&& levelMode = std::equal_to<unsigned int>(), std::optional<std::string>&& property = std::nullopt);
		const ArmorSet& build() const {
			return *this;
		}
		bool ensure_equipped(nlohmann::json& character, const LightSocket& wrapper) const;
	};
}

#endif