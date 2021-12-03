#pragma once

#ifndef MESSAGE_ENUM_HPP_
#define MESSAGE_ENUM_HPP_

namespace MessageEnum {
	enum MESSAGE_TYPE {
		// CM_MESSAGE = 1,
		// SERVER_MESSAGE = 2,
		// Currently only IPC (Inter Process Communication or Inter Process Channel) messages are implemented.
		IPC_MESSAGE = 0
	};
}

#endif /* MESSAGE_ENUM_HPP_ */