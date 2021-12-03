#include "Service.hpp"
#include <memory> 
#include <thread>
#include <iostream>

long ipc_handler(AddArguments* arguments) {
	return arguments->first_num + arguments->second_num;
}

extern "C" void* init(void* id) {
	return (void*) &ipc_handler;
}
