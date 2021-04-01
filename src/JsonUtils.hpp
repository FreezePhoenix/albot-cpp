#pragma once

#ifndef JSONUTILS_HPP_
#define JSONUTILS_HPP_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <iostream>
#ifndef INCLUDE_NLOHMANN_JSON_HPP_
	#include <nlohmann/json.hpp>
#endif

namespace JsonUtils {
	void parse(rapidjson::Document* _json, std::string* str);
	void stripComments(std::string *str);
};

#endif /* JSONUTILS_HPP_ */
