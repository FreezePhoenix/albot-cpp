#pragma once

#ifndef SOCKETWRAPPER_HPP_
#define SOCKETWRAPPER_HPP_

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <chrono>
#include "Bot.hpp"
#include "HttpWrapper.hpp"
#include <functional>
#include <algorithm>
#include <regex>

typedef std::function<void(const ix::WebSocketMessagePtr&)> RawCallback;
typedef std::function<void(const nlohmann::json&)> EventCallback;

class ReconnectInfo {
	public:
		int wait;
		long spawn;

		ReconnectInfo() : wait(0), spawn(0) {
		}

		void create(int wait) {
			spawn = std::chrono::duration_cast < std::chrono::milliseconds > (std::chrono::system_clock::now().time_since_epoch()).count();
			this->wait = wait * 1000;

		}

		void reconnecting() {
			this->wait = 0;
			this->spawn = 0;
		}

};

class SocketWrapper {
	private:
		static auto inline const mLogger = spdlog::stdout_color_mt("SocketWrapper");
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

		std::mutex chestGuard;
		std::mutex deletionGuard;
		std::mutex entityGuard;

		std::map<std::string, nlohmann::json> chests;

		void triggerInternalEvents(std::string eventName, const nlohmann::json &event);
		void dispatchEvent(std::string eventName, const nlohmann::json &event);
		void messageReceiver(const ix::WebSocketMessagePtr &message);
		void initializeSystem();
		void login(GameInfo* info);

		/**
		 * Cleans up input to avoid type bugs introduced by the backend.
		 * One notable use for this is with the `rip` attribute.
		 * Some chars have an integer value as the value, but this
		 * client expects a bool, meaning anything else will trigger a
		 * crash.
		 */
		void sanitizeInput(nlohmann::json &entity);
	public:
		ReconnectInfo reconn = { };
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
		void registerEventCallback(std::string event, std::function<void(const nlohmann::json&)> callback);
		void deleteEntities();

		void receiveLocalCm(std::string from, const nlohmann::json &message);
		/**
		 *  Connects a user. this should only be run from the Player class
		 */
		void connect();
		void reconnect();

		void close();
		void sendPing();
		void emit(std::string event, const nlohmann::json &json = { });

		void onDisappear(const nlohmann::json &event);

		void changeServer(HttpWrapper::Server *server);

		std::map<std::string, nlohmann::json>& getEntities();
		std::map<std::string, nlohmann::json>& getUpdateEntities();
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

#endif /* SOCKETWRAPPER_HPP_ */
