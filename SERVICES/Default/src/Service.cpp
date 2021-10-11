#include "Service.hpp"
#include "../../src/ServiceInterface.hpp"
#include <memory> 
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

static std::shared_ptr<spdlog::logger> mLogger = spdlog::stdout_color_mt<spdlog::async_factory>("DefaultService");

void* ipc_handler(Message* message) {
	AddArguments* arguments = (AddArguments*)message->arguments;
	mLogger->info("Service called! Requested to add: {} and {}", arguments->first_num, arguments->second_num);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	return (void*)(arguments->first_num + arguments->second_num);
}

extern "C" void* init(void* id) {
	return (void*) &ipc_handler;
}
