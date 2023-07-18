#ifndef BOTIMPL_ARMORMANAGER_HPP_
#define BOTIMPL_ARMORMANAGER_HPP_

#include "LightSocket.hpp"
#include <nlohmann/json.hpp>
#include <optional>
#include <functional>

namespace ArmorManager {
	extern size_t char_stamp;
	struct ArmorItemFilter {
		const std::string slot;
		const std::string name;
		const std::optional<unsigned int> level;
		const std::optional<std::string> property;
		bool test(const nlohmann::json& item) const;
	};

	struct ArmorSet {
	private:
		mutable size_t stamp = -1;
		std::vector<ArmorItemFilter> item_filters;
	public:
		ArmorSet& add_item(std::string&& slot, std::string&& name, std::optional<unsigned int>&& level = std::nullopt, std::optional<std::string>&& property = std::nullopt);
		const ArmorSet& build() const {
			return *this;
		}
		void attempt_equip(const LightSocket& wrapper) const;
	};
}

#endif