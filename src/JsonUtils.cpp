#include "JsonUtils.hpp"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "fmt/os.h"

namespace JsonUtils {
    void parse(rapidjson::Document* _json, std::string* str)  {
		_json->Parse(str->c_str());
	};
    void strip_comments(std::string *str)  {
		rapidjson::Document _json;
		_json.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(str->c_str());
		if (_json.HasParseError()) {
			fmt::print("{}", _json.GetParseError());
			fmt::print("{}", _json.GetErrorOffset());
		}
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		_json.Accept(writer);
		const char* output = buffer.GetString();
		str->assign(output);
	};
}