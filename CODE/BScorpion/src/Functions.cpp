#include "Functions.hpp"

bool Functions::needs_mp(const nlohmann::json& entity) {
	auto mp_it = entity.find("mp");
	if(mp_it == entity.end()) {
		return false;
	}
	auto max_mp_it = entity.find("max_mp");
	if(max_mp_it == entity.end()) {
		return false;
	}
	return double(*mp_it) / double(*max_mp_it) < 0.75;
}

double Functions::distance(const nlohmann::json& A, const nlohmann::json& B) {
	if(A.contains("x") && A.contains("y") && B.contains("x") && B.contains("y")) {
		return std::hypot(A["x"].get<double>() - B["x"].get<double>(), A["y"].get<double>() - B["y"].get<double>());
	}
	return NAN;
};

double Functions::distance_squared(const nlohmann::json& A, const nlohmann::json& B) {
	if(A.contains("x") && A.contains("y") && B.contains("x") && B.contains("y")) {
		double dx = double(A["x"]) - double(B["x"]);
		double dy = double(A["y"]) - double(B["y"]);
		return dx * dx + dy * dy;
	}
	return NAN;
};

bool Functions::needs_hp(const nlohmann::json& entity) {
	auto hp_it = entity.find("hp");
	if(hp_it == entity.end()) {
		return false;
	}
	auto max_hp_it = entity.find("max_hp");
	if(max_hp_it == entity.end()) {
		return false;
	}
	return double(*hp_it) / double(*max_hp_it) < 0.75;
}
