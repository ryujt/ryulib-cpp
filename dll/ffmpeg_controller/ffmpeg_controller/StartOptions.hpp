#pragma once

#include <Windows.h>
#include <string>
#include <nlohmann/json.hpp>
#include <ryulib/debug_tools.hpp>
#include <ryulib/disk.hpp>
#include <ryulib/strg.hpp>

using namespace std;
using namespace nlohmann;

using json = nlohmann::json;

class StartOption {
public:
	bool parseJson(string option)
	{
		try {
			json_ = json::parse(AnsiToUTF8(option));
		} catch (...) {
			json_ = nullptr;
			return false;
		}
		return true;
	}

	int get_int(string name, int default_value)
	{
		try {
			return json_[name].get<int>();
		} catch (...) {
			return default_value;
		}
	}

	bool get_bool(string name, bool default_value)
	{
		try {
			return json_[name].get<bool>();
		} catch (...) {
			return default_value;
		}
	}

	float get_float(string name, float default_value)
	{
		try {
			return json_[name].get<float>();
		} catch (...) {
			return default_value;
		}
	}

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