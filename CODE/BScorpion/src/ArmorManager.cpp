#include "ArmorManager.hpp"

#include <algorithm>

bool ArmorManager::ArmorItemFilter::test(const nlohmann::json& item) const {
	if (item.is_null()) {
		return false;
	}
	if (level.has_value()) {
		if (!levelMode(item.value("level", 0U), level.value())) {
			return false;
		}
	}
	if (item["name"].get<std::string>() != name) {
		return false;
	}
	if (property.has_value()) {
		if (item.contains("p")) {
			if (item["p"].get<std::string>() != property.value()) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}


ArmorManager::ArmorSet& ArmorManager::ArmorSet::add_item(std::string&& slot, std::string&& name, std::optional<unsigned int>&& level, std::function<bool(unsigned int, unsigned int)>&& levelMode, std::optional<std::string>&& property) {
	item_filters.emplace_back(std::move(slot), std::move(name), std::move(level), std::move(levelMode), std::move(property));
	return *this;
}


bool ArmorManager::ArmorSet::ensure_equipped(nlohmann::json& character, const LightSocket& wrapper) const {
	std::vector<bool> FLAGS(item_filters.size());
	auto& slots = character["slots"];
	for (size_t i = 0; i < item_filters.size(); i++) {
		const auto& filter = item_filters[i];
		if (filter.test(slots[filter.slot])) {
			FLAGS[i] = true;
		}
	}
	auto& items = character["items"];
	for (size_t i = 0; i < items.size(); i++) {
		auto& item = items[i];
		if (item.is_null()) {
			continue;
		}
		for (size_t j = 0; j < item_filters.size(); j++) {
			if (!FLAGS[j]) {
				const auto& filter = item_filters[j];
				if (filter.test(item)) {
					nlohmann::json& equipped = slots[filter.slot];
					wrapper.emit("equip", {
						{"num", i},
						{"slot", filter.slot}
					});
					// Swap the items on the client...
					std::swap(item, equipped);
					FLAGS[j] = true;
				}
			}
		}
	}
	return std::all_of(FLAGS.begin(), FLAGS.end(), std::identity());
}

