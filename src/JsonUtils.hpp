#pragma once

#ifndef JSONUTILS_HPP_
#define JSONUTILS_HPP_

#include <rapidjson/document.h>

namespace JsonUtils {
	void parse(rapidjson::Document* _json, std::string* str);
	void strip_comments(std::string *str);
};

#endif /* JSONUTILS_HPP_ */
