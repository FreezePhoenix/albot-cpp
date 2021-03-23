#pragma once

#ifndef JSONUTILS_HPP_
#define JSONUTILS_HPP_

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

class JsonUtils {
public:
	static void parse(rapidjson::Document* _json, std::string* str)  {
		_json->Parse(str->c_str());
	};
	static void stripComments(std::string *str)  {
		rapidjson::Document _json;
		_json.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(str->c_str());
		if(_json.HasParseError()) {
			std::cout << _json.GetParseError() << std::endl;
			std::cout << _json.GetErrorOffset() << std::endl;
		}
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		_json.Accept(writer);
		const char* output = buffer.GetString();
		str->assign(output);
	};
};

#endif /* JSONUTILS_HPP_ */
