#ifndef ALBOT_SERVICE_INTERFACE_HPP_
#define ALBOT_SERVICE_INTERFACE_HPP_

#include <string>

struct Message {
	std::string command;
	std::string requester;
	std::string target;
	void* arguments;
};

#endif /* ALBOT_SERVICE_INTERFACE_HPP_ */