#pragma once

#ifndef JSONUTILS_HPP_
#define JSONUTILS_HPP_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "sio_message.h"
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

class JsonUtils {
public:
	static void parse(rapidjson::Document* json, std::string* str);
	static sio::message::ptr createObject(nlohmann::json o);
	static sio::message::ptr createArray(nlohmann::json o);
	static void stripComments(std::string *str);
};

#endif /* JSONUTILS_HPP_ */
