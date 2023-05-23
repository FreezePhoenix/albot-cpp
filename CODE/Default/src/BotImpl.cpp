#include <iostream>
#include <string>
#include <mutex>

#include "albot/SocketWrapper.hpp"
#include "albot/Utils/LoopHelper.hpp"
#include "albot/Utils/Timer.hpp"
#include "albot/MovementMath.hpp"
#include "albot/Utils/ParsingUtils.hpp"

#include "albot/albot-cpp.hpp"
#include "../../SERVICES/Pathfinding/include/Pathfinding/Service.hpp"

#ifndef CHARACTER_NAME
	#define CHARACTER_NAME	-1
#endif

#ifndef CHARACTER_CLASS
	#define CHARACTER_CLASS -1
#endif
// extern std::map<std::string, ServiceInfo<void*, void>*> ALBot::SERVICE_HANDLERS;
const Types::TimePoint epoch;

template<typename RESULT, typename ARGUMENTS>
RESULT invoke_service(const std::string& name, ARGUMENTS* arguments) {
	return ALBot::invoke_service<ARGUMENTS, RESULT>(name, arguments);
}

class BotImpl : public Bot {
	public:
		std::promise<std::thread*> loop_thread = std::promise<std::thread*>();
		LoopHelper loop;
		Types::TimePoint last;
		std::shared_ptr<SocketWrapper> wrapper;
		bool running = true;
		std::thread uvThread;
		std::shared_ptr<spdlog::logger> mLogger;
		BotImpl(const CharacterGameInfo& id) : Bot(id), loop() {
			this->wrapper = std::shared_ptr<SocketWrapper>(new SocketWrapper(std::to_string(info.character->id), this->info.server->url, *this));
			this->mLogger = spdlog::stdout_color_mt(this->info.character->name + ":BotImpl");
			this->name = info.character->name;
			this->id = info.character->id;
			loop.setInterval([this](const uvw::TimerEvent&, uvw::TimerHandle&) {
				this->processInternals();
			}, 1000.0 / 60.0);
			loop.setInterval([this](const uvw::TimerEvent&, uvw::TimerHandle&) {
				for (auto& [id, entity] : wrapper->getEntities()) {
					mLogger->info("{} has {} hp", id, entity.at("hp").get<int>());
				}
			}, 1000.0);
			loop.setTimeout([this](const uvw::TimerEvent&, uvw::TimerHandle&) {
				this->stop();
				this->running = false;
			}, 5000);
		};
		void processInternals() {
			if (last == epoch) last = Types::Clock::now();

			std::map<std::string, nlohmann::json> updateEntities;

			{
				// The intermediate map is used to reduce the amount of data races.
				// Now, it's only between the socket, and this function. This specific
				// tiny code snippet uses a mutex, that interacts with the inserting
				// and updating function (entities event). This blocks changes. 
				// Additionally, a copy is stored in this function, which lets the socket
				// continue processing right after the copy and clearing of the previous
				// entities have been handled
				// This function is also run from a loop, which is blocking in terms of 
				// other loops and timers. This means while this function is processing, 
				// no loops will be accessing the entities map. 
				// Thread safety first, kids!
				std::lock_guard<std::mutex> lock(this->wrapper->getEntityGuard());
				updateEntities = wrapper->getUpdateEntities();   
				wrapper->getUpdateEntities().clear();
			}

			wrapper->deleteEntities();

			auto& entities = wrapper->getEntities();
			for (auto& [id, data] : updateEntities) {
				if (entities.find(id) != entities.end()) {
					entities[id].update(data);
				} else entities[id] = data;
			}

			auto now = Types::Clock::now();

			const double delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
			last = now;
			double cDelta = delta;

			while (cDelta > 0) {
				if (this->isAlive() && this->isMoving()) {

					nlohmann::json& entity = this->getRawJson();
					if (entity.find("ref_speed") == entity.end() ||
						(entity.find("ref_speed") != entity.end() && entity["ref_speed"] != entity["speed"])) {
						entity["ref_speed"] = entity["speed"];
						entity["from_x"] = entity["x"];
						entity["from_y"] = entity["y"];
						std::pair<int, int> vxy = MovementMath::calculateVelocity(entity);
						entity["vx"] = vxy.first;
						entity["vy"] = vxy.second;
						entity["engaged_move"] = entity["move_num"];
					}
					MovementMath::moveEntity(entity, cDelta);
					MovementMath::stopLogic(entity);
				}

				for (auto& [id, entity] : wrapper->getEntities()) {

					if (entity.find("speed") == entity.end() && entity["type"] == "monster") {
						std::string type = entity["mtype"];
						entity["speed"] = this->info.G->getData()["monsters"][type]["speed"].get<double>();
					}
					if (!getOrElse(entity, "rip", false) && !getOrElse(entity, "dead", false) &&
						getOrElse(entity, "moving", false)) {
						if (entity.value("move_num", 0l) != entity.value("engaged_move", 0l) ||
							(entity.find("ref_speed") != entity.end() && entity["ref_speed"] != entity["speed"])) {
							entity["ref_speed"] = entity["speed"];
							entity["from_x"] = entity["x"];
							entity["from_y"] = entity["y"];
							std::pair<int, int> vxy = MovementMath::calculateVelocity(entity);
							entity["vx"] = vxy.first;
							entity["vy"] = vxy.second;

							entity["engaged_move"] = entity["move_num"];
						}

						MovementMath::moveEntity(entity, cDelta);
						MovementMath::stopLogic(entity); // Processes whether we're done moving or not.
					}
				}

				cDelta -= 50;
			}
		}
		void startUVThread() {
			running = true;
			uvThread = std::thread([this]() {
				while (running) {
					loop.getLoop()->run<uvw::Loop::Mode::ONCE>();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			});
		}
		void onCm(const std::string& name, const nlohmann::json& data) {
			mLogger->info("{} sent {}", name, data.dump());
		}
		void onConnect() {
			this->log("Connected!?!");
			// Say hello!
			// this->wrapper->emit("say", { {"message", "UwU drippy"} });
			this->startUVThread();
			loop_thread.set_value(&uvThread);
		}
		void start() {
			wrapper->connect();
		}
		void stop() {
			wrapper->close();
		}
};

BotImpl* BotInstance;

void ipc_handler(Message* message) {
	if (message->command == "code_message") {
		BotInstance->onCm(message->requester, *((nlohmann::json*)message->arguments));
	} else if (message->command == "code_message_fail") {
		BotInstance->mLogger->error("Sad face :c");
	}
}

void cleanup() {
	BotInstance->log("DESTRUCTING");
	delete BotInstance;
}

extern "C" void init(CharacterGameInfo& info) {
	info.child_handler = &ipc_handler;
	info.destructor = cleanup;
	BotInstance = new BotImpl(info);
	auto path = ALBot::invoke_service<PathfindArguments, PathfindArguments::PathResult*>("Pathfinding", PathfindArguments{ PathfindArguments::Point{ 778,-506}, PathfindArguments::Point{ -700,906 }, "main", "main" });
	std::cout << path->path.size() << std::endl;
	for (PathfindArguments::Point point : path->path) {
		BotInstance->log(std::to_string(point.x) + "," + std::to_string(point.y));
	}
	BotInstance->log("Class: " + ClassEnum::getClassStringInt(CHARACTER_CLASS));
	BotInstance->log("Logging in... ");
	BotInstance->start();
	BotInstance->loop_thread.get_future().get()->join();
}