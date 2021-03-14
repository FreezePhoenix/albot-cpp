#include "./JsonUtils.hpp"

using namespace std;
using namespace rapidjson;
using namespace nlohmann;

void JsonUtils::parse(Document* json, string* str) {
	json->Parse(str->c_str());
}
sio::message::ptr JsonUtils::createObject(json o) {
	sio::message::ptr object = object_message::create();

	for (json::iterator it = o.begin(); it != o.end(); ++it) {
		auto key = it.key();
		auto v = it.value();
		if (v.is_boolean()) {
			object->get_map()[key] = bool_message::create(v.get<bool>());
		}
		else if (v.is_number_integer()) {
			object->get_map()[key] = int_message::create(v.get<int>());
		}
		else if (v.is_string()) {
			object->get_map()[key] = string_message::create(v.get<std::string>());
		}
		else if (v.is_array()) {
			json childObject = v;
			object->get_map()[key] = createArray(v);
		}
		else if (v.is_object()) {
			json childObject = v;
			object->get_map()[key] = createObject(childObject);
		}
	}
	return object;
}

sio::message::ptr JsonUtils::createArray(json o) {
	sio::message::ptr array = array_message::create();

	for (json::iterator it = o.begin(); it != o.end(); ++it) {

		auto v = it.value();
		if (v.is_boolean()) {
			array->get_vector().push_back(bool_message::create(v.get<bool>()));
		}
		else if (v.is_number_integer()) {
			array->get_vector().push_back(int_message::create(v.get<int>()));
		}
		else if (v.is_string()) {
			array->get_vector().push_back(string_message::create(v.get<string>()));
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

void JsonUtils::stripComments(string *str) {
	Document _json;
	_json.Parse<kParseCommentsFlag | kParseTrailingCommasFlag>(str->c_str());
	if(_json.HasParseError()) {
		cout << _json.GetParseError() << endl;
		cout << _json.GetErrorOffset() << endl;
	}
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	_json.Accept(writer);
	const char* output = buffer.GetString();
	str->assign(output);
}
