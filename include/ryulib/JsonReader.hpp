#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <ryulib/strg.hpp>

using namespace std;
using namespace nlohmann;

using json = nlohmann::json;

/** ������ Json ���ڿ��� �Ľ��Ͽ� ���� �����´�. */
class JsonReader {
public:
	/** ���ڿ��� �о�ͼ� �Ľ��Ѵ�. */
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

	/** Ansi ���ڿ��� �о�ͼ� �Ľ��Ѵ�. */
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

	/** ������ name�� �ش��ϴ� ���� ���� �д´�.
	@param name �а����ϴ� name ��
	@param default_value json ���ڿ� �ȿ� name�� ������ default_value�� �����Ѵ�.
	*/
	int get_int(string name, int default_value)
	{
		try {
			return json_[name].get<int>();
		} catch (...) {
			return default_value;
		}
	}

	/** ������ name�� �ش��ϴ� �Ҹ� ���� �д´�.
	@param name �а����ϴ� name ��
	@param default_value json ���ڿ� �ȿ� name�� ������ default_value�� �����Ѵ�.
	*/
	bool get_bool(string name, bool default_value)
	{
		try {
			return json_[name].get<bool>();
		} catch (...) {
			return default_value;
		}
	}

	/** ������ name�� �ش��ϴ� float ���� �д´�.
	@param name �а����ϴ� name ��
	@param default_value json ���ڿ� �ȿ� name�� ������ default_value�� �����Ѵ�.
	*/
	float get_float(string name, float default_value)
	{
		try {
			return json_[name].get<float>();
		} catch (...) {
			return default_value;
		}
	}

	/** ������ name�� �ش��ϴ� ���ڿ��� �д´�.
	@param name �а����ϴ� name ��
	@param default_value json ���ڿ� �ȿ� name�� ������ default_value�� �����Ѵ�.
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