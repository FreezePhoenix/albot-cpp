#pragma once

#ifndef ALBOT_SOCKETWRAPPER_HPP_
#define ALBOT_SOCKETWRAPPER_HPP_

#include <ixwebsocket/IXWebSocket.h>

#include <nlohmann/json.hpp>

#include <vector>

#include <mutex>
#include <map>
#include <chrono>

#include "albot/Bot.hpp"

#include <functional>


typedef std::function<void(const ix::WebSocketMessagePtr&)> RawCallback;
typedef std::function<void(const nlohmann::json&)> EventCallback;

class SocketWrapper {
	private:
		std::shared_ptr<spdlog::logger> mLogger;
		static std::string inline const waitRegex = "wait_(\\d+)_seconds";

		ix::WebSocket webSocket;
		Bot &player;
		std::string characterId;
		// ping managing
		int pingInterval;
		std::chrono::time_point<std::chrono::high_resolution_clock> lastPing;

		// Entity management
		bool hasReceivedFirstEntities;

		// Callbacks
		std::vector<RawCallback> rawCallbacks;
		std::map<std::string, std::vector<EventCallback>> eventCallbacks;

		std::map<std::string, nlohmann::json> entities;
		std::map<std::string, nlohmann::json> updatedEntities;

		
		nlohmann::json character;
		nlohmann::json updatedCharacter;

		std::mutex chestGuard;
		std::mutex deletionGuard;
		std::mutex entityGuard;

		std::map<std::string, nlohmann::json> chests;
		void handle_entities(const nlohmann::json&);
		void triggerInternalEvents(std::string eventName, const nlohmann::json &event);
		void dispatchEvent(std::string eventName, const nlohmann::json &event);
		void messageReceiver(const ix::WebSocketMessagePtr &message);
		void initializeSystem();
		void login(const CharacterGameInfo& info);

		/**
		 * Cleans up input to avoid type bugs introduced by the backend.
		 * One notable use for this is with the `rip` attribute.
		 * Some chars have an integer value as the value, but this
		 * client expects a bool, meaning anything else will trigger a
		 * crash.
		 */
		void sanitizeInput(nlohmann::json &entity);
	public:
		/**
		 * Initializes a general, empty SocketWrapper ready to connect.
		 * When {@link #connect(std::string)} is called, the socket starts up
		 * and runs the connection procedures for the target server, the
		 * newly created SocketWrapper will be designated to a specific in-game
		 * character.
		 */
		SocketWrapper(std::string characterId, std::string fullUrl, Bot &player);
		~SocketWrapper();

		/**
		 * Registers a listener that receives raw events straight from the websocket. These aren't
		 * parsed - at all - and the receiving function is expected to do so.
		 *
		 * This only sends messages - other status codes aren't sent.
		 */
		void registerRawMessageCallback(std::function<void(const ix::WebSocketMessagePtr&)> callback);
		/**
		 * Equivalent of socket.on
		 */
		void registerEventCallback(const std::string& event, std::function<void(const nlohmann::json&)> callback);
		void deleteEntities();

		void receiveLocalCm(std::string from, const nlohmann::json &message);
		/**
		 *  Connects a user. this should only be run from the Player class
		 */
		void connect();

		void close();
		void sendPing();
		void emit(const std::string& event, const nlohmann::json &json = { });
		void emitRawJsonString(std::string event, std::string json = " ");
		void onDisappear(const nlohmann::json &event);

		void changeServer(Server *server);

		std::map<std::string, nlohmann::json>& getEntities();
		std::map<std::string, nlohmann::json>& getUpdateEntities();

		nlohmann::json& getCharacter();
		nlohmann::json& getUpdateCharacter();
		
		std::map<std::string, nlohmann::json>& getChests();

		bool isOpen() {
			return webSocket.getReadyState() == ix::ReadyState::Open;
		}
		ix::ReadyState getReadyState() {
			return webSocket.getReadyState();
		}

		std::mutex& getChestGuard() {
			return chestGuard;
		}
		std::mutex& getEntityGuard() {
			return entityGuard;
		}
};

#endif /* ALBOT_SOCKETWRAPPER_HPP_ */
