#include "JsonUtils.hpp"

void JsonUtils::parse(rapidjson::Document* _json, std::string* str) {
	_json->Parse(str->c_str());
}
sio::message::ptr JsonUtils::createObject(nlohmann::json o) {
	sio::message::ptr object = sio::object_message::create();
	for (nlohmann::json::iterator it = o.begin(); it != o.end(); ++it) {
		auto key = it.key();
		auto v = it.value();
		if (v.is_boolean()) {
			object->get_map()[key] = sio::bool_message::create(v.get<bool>());
		}
		else if (v.is_number_integer()) {
			object->get_map()[key] = sio::int_message::create(v.get<int>());
		}
		else if (v.is_string()) {
			object->get_map()[key] = sio::string_message::create(v.get<std::string>());
		}
		else if (v.is_array()) {
			nlohmann::json childObject = v;
			object->get_map()[key] = createArray(v);
		}
		else if (v.is_object()) {
			nlohmann::json childObject = v;
			object->get_map()[key] = createObject(childObject);
		}
	}
	return object;
}

sio::message::ptr JsonUtils::createArray(nlohmann::json o) {
	sio::message::ptr array = sio::array_message::create();

	for (nlohmann::json::iterator it = o.begin(); it != o.end(); ++it) {

		auto v = it.value();
		if (v.is_boolean()) {
			array->get_vector().push_back(sio::bool_message::create(v.get<bool>()));
		}
		else if (v.is_number_integer()) {
			array->get_vector().push_back(sio::int_message::create(v.get<int>()));
		}
		else if (v.is_string()) {
			array->get_vector().push_back(sio::string_message::create(v.get<std::string>()));
		}
		else if (v.is_array()) {
			array->get_vector().push_back(createArray(v));
		}
		else if (v.is_object()) {
			array->get_vector().push_back(createObject(v));
		}
	}
	return array;
}

void JsonUtils::stripComments(std::string *str) {
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
}
