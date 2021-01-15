#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <ryulib/strg.hpp>

using namespace std;
using namespace nlohmann;

using json = nlohmann::json;

/** 간단한 Json 문자열을 파싱하여 값을 가져온다. */
class JsonReader {
public:
	/** 문자열을 읽어와서 파싱한다. */
	bool loadText(string option)
	{
		try {
			json_ = json::parse(option);
		} catch (...) {
			json_ = nullptr;
			return false;
		}
		return true;
	}

	/** Ansi 문자열을 읽어와서 파싱한다. */
	bool loadAnsiText(string option)
	{
		try {
			json_ = json::parse(AnsiToUTF8(option));
		} catch (...) {
			json_ = nullptr;
			return false;
		}
		return true;
	}

	/** 지정된 name에 해당하는 정수 값을 읽는다.
	@param name 읽고자하는 name 값
	@param default_value json 문자열 안에 name이 없으면 default_value를 리턴한다.
	*/
	int get_int(string name, int default_value)
	{
		try {
			return json_[name].get<int>();
		} catch (...) {
			return default_value;
		}
	}

	/** 지정된 name에 해당하는 불린 값을 읽는다.
	@param name 읽고자하는 name 값
	@param default_value json 문자열 안에 name이 없으면 default_value를 리턴한다.
	*/
	bool get_bool(string name, bool default_value)
	{
		try {
			return json_[name].get<bool>();
		} catch (...) {
			return default_value;
		}
	}

	/** 지정된 name에 해당하는 float 값을 읽는다.
	@param name 읽고자하는 name 값
	@param default_value json 문자열 안에 name이 없으면 default_value를 리턴한다.
	*/
	float get_float(string name, float default_value)
	{
		try {
			return json_[name].get<float>();
		} catch (...) {
			return default_value;
		}
	}

	/** 지정된 name에 해당하는 문자열을 읽는다.
	@param name 읽고자하는 name 값
	@param default_value json 문자열 안에 name이 없으면 default_value를 리턴한다.
	*/
	string get_string(string name, string default_value)
	{
		try {
			return json_[name].get<string>();
		} catch (...) {
			return default_value;
		}
	}

private:
	json json_;
};