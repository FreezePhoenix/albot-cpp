#pragma once

#ifndef ALBOT_SERVICE_INTERFACE_HPP_
#define ALBOT_SERVICE_INTERFACE_HPP_
#include <iostream>
#include "albot/Enums/ClassEnum.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <functional>
#include <any>

class MutableGameData {
private:
	std::shared_ptr<nlohmann::json> data;
public:
	MutableGameData(std::string& rawJson) : data(new nlohmann::json()) {
		// Create a new JSON object.
		nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
	}
	MutableGameData(std::istream& rawJson) : data(new nlohmann::json()) {
		// Create a new JSON object.
		nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
	}
	MutableGameData(const MutableGameData& old) : data(old.data) {
	};

	nlohmann::json& getData() {
		return *data;
	}
	nlohmann::json& operator[](const std::string& key) {
		return data->operator[](key);
	}
	nlohmann::json& at(const std::string& key) {
		return data->at(key);
	}

	friend class GameData;
};
class GameData {
private:
	std::shared_ptr<nlohmann::json> data;
public:
	bool was_cached = false;
	GameData() : data(new nlohmann::json()) {
	}
	GameData(const std::string& rawJson) : data(new nlohmann::json()) {
		was_cached = false;
		nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
	}
	GameData(std::istream& rawJson) : data(new nlohmann::json()) {
		was_cached = true;
		nlohmann::json::parse(rawJson, nullptr, true, true).swap(*this->data);
	}
	GameData(const GameData& old) : data(old.data) {
		was_cached = true;
	}
	GameData(const MutableGameData& old) : data(old.data) {
		was_cached = false;
	}
	GameData(MutableGameData&& old) : data(std::move(old.data)) {
		was_cached = false;
	}

	// Utility methods to access the JSON.
	const nlohmann::json& getData() const {
		return *data;
	}
	const nlohmann::json& operator[](const std::string& key) const {
		return data->at(key);
	}
	bool contains(const std::string& key) const {
		return data->contains(key);
	}
	const nlohmann::json& at(const std::string& key) const {
		return data->at(key);
	}
};

struct Character {
	std::string name;
	size_t id;
	bool enabled;
	ClassEnum::CLASS klass;
	std::string script;
	std::string server;
};
struct Server {
	std::string region;
	int port;
	bool pvp;
	std::string ip;
	std::string identifier;
	std::string url;
	std::string fullName;
};

template<template<typename... Args> typename T>
struct erased_type {
private:
	std::any internal;
public:
	erased_type() : internal() {
	}
	template<typename... Args>
	erased_type(const T<Args...>& obj) : internal(obj) {
	}
	template<typename... Args>
	erased_type(T<Args...>&& obj) : internal(obj) {
	}
	template<typename... Args>
	erased_type& operator=(const T<Args...>& obj) {
		internal = obj;
		return *this;
	}
	template<typename... Args>
	erased_type& operator=(T<Args...>&& obj) {
		internal = std::move(obj);
		return *this;
	}
	template<typename... Args>
	T<Args...> get() {
		return std::any_cast<T<Args...>>(internal);
	}
	bool has_value() const noexcept {
		return internal.has_value();
	}
};

class ServiceInfo {
private:
	erased_type<std::function> handler;
	std::function<void()> destructor;
public:
	const GameData& G;
	ServiceInfo(const GameData& data) : G(data) {
		
	};
	void set_destructor(std::function<void()> destructor) {
		this->destructor = destructor;
	}
	template<typename R, typename... Args>
	void set_handler(std::function<R(Args...)> new_handler) {
		this->handler = new_handler;
	}
	template<typename R, typename... Args>
	std::function<R(Args...)> get_handler() {
		return this->handler.get<R(Args...)>();
	}
	bool has_destructor() {
		return (bool)destructor;
	}
	bool has_handler() {
		return handler.has_value();
	}
	~ServiceInfo() {
		if (destructor) {
			destructor();
		}
	}
};

#endif /* ALBOT_SERVICE_INTERFACE_HPP_ */