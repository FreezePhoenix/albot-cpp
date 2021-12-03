#pragma once

#ifndef ALBOT_JSONUTILS_HPP_
#define ALBOT_JSONUTILS_HPP_

#include <rapidjson/document.h>

namespace JsonUtils {
	void parse(rapidjson::Document* _json, std::string* str);
	void strip_comments(std::string *str);
};

#endif /* ALBOT_JSONUTILS_HPP_ */
