#ifndef JSONUTILS_HPP_
#define JSONUTILS_HPP_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "sio_client.h"
#include "sio_socket.h"
#include "sio_socket.h"
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace rapidjson;
using namespace sio;
using namespace nlohmann;

class JsonUtils {
public:
	static void parse(Document* json, string* str);
	static sio::message::ptr createObject(json o);
	static sio::message::ptr createArray(json o);
	static void stripComments(string *str);
};

#endif /* JSONUTILS_HPP_ */
