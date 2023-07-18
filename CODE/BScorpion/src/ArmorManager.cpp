#include "ArmorManager.hpp"

#include <algorithm>

size_t ArmorManager::char_stamp = 0;

bool ArmorManager::ArmorItemFilter::test(const nlohmann::json& item) const {
	if (item.is_null()) {
		return false;
	}
	
	if(item.contains("name")) {
		if (item["name"].get<std::string>() != name) {
			return false;
		}
	} else {
		return false;
	}
	
	if (level.has_value()) {
		if(item.contains("level")) {
			if(item["level"].get<unsigned int>() != level.value()) {
				return false;
			}
		} else {
			return false;
		}
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

ArmorManager::ArmorSet& ArmorManager::ArmorSet::add_item(std::string&& slot, std::string&& name, std::optional<unsigned int>&& level, std::optional<std::string>&& property) {
	item_filters.emplace_back(std::move(slot), std::move(name), std::move(level), std::move(property));
	return *this;
}


void ArmorManager::ArmorSet::attempt_equip(const LightSocket& wrapper) const {
	if(ArmorManager::char_stamp == stamp) {
		return;
	}
	auto& character = wrapper.character();
	auto& slots = character["slots"];
	auto& items = character["items"];
	for(const auto& filter : item_filters) {
		if(!slots.contains(filter.slot)) {
			continue;
		}
		auto& slot = slots[filter.slot];
		
		if (!filter.test(slot)) {
			for (size_t i = 0; i < items.size(); i++) {
				auto& item = items[i];
				if (filter.test(item)) {
					wrapper.emit("equip", {
						{"num", i},
						{"slot", filter.slot}
					});
					// Swap the items on the client...
					std::swap(item, slot);
					break;
				}
			}
		}
	}
	
	stamp = ++ArmorManager::char_stamp;
}

