#include "files.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>

#include "../logger/Logger.h"
#include "../coders/json.h"
#include "../util/stringutil.h"

files::rafile::rafile(std::filesystem::path filename) : file(filename, std::ios::binary | std::ios::ate) {
    if (!file) {
		LOG_ERROR("Could not to open file '{}'", filename.string());
		Logger::getInstance().flush();
        throw std::runtime_error("Could not to open file " + filename.string());
    }
    filelength = file.tellg();
    file.seekg(0);
}

size_t files::rafile::length() const {
    return filelength;
}

void files::rafile::seekg(std::streampos pos) {
    file.seekg(pos);
}

void files::rafile::read(char* buffer, std::streamsize size) {
    file.read(buffer, size);
}

// Записывает данные в бинарный файл (перезаписывает сущесвующий)
bool files::write_bytes(const std::filesystem::path filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary);
	if (!output.is_open()) return false;

	output.write(data, size);
	output.close();

	return true;
}

// Добавляет данные в конец бинарного файла
uint files::append_bytes(const std::filesystem::path filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary | std::ios::app);
	if (!output.is_open()) return 0;

	uint position = output.tellp();
	output.write(data, size);
	output.close();

	return position;
}

// Читает данные из бинарного файла с начала
bool files::read(const std::filesystem::path filename, char* data, size_t size) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return false;

	input.read(data, size);
	input.close();

	return true;
}

char* files::read_bytes(std::filesystem::path filename, size_t& length) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return nullptr;
	input.seekg(0, std::ios_base::end);
	length = input.tellg();
	input.seekg(0, std::ios_base::beg);

	std::unique_ptr<char> data(new char[length]);
	input.read(data.get(), length);
	input.close();
	return data.release();
}

std::string files::read_string(std::filesystem::path filename) {
	size_t size;
	std::unique_ptr<char> chars (read_bytes(filename, size));
	if (chars == nullptr) {
		LOG_ERROR("Could not to load file '{}'", filename.string());
		throw std::runtime_error("Could not to load file '" + filename.string() + "'");
	}
	return std::string(chars.get(), size);
}

bool files::write_string(std::filesystem::path filename, const std::string content) {
	std::ofstream file(filename);
	if (!file) return false;

	file << content;
	return true;
}

bool files::write_json(std::filesystem::path filename, const json::JObject* obj, bool nice) {
    return files::write_string(filename, json::stringify(obj, nice, "  "));
}

bool files::write_binary_json(std::filesystem::path filename, const json::JObject* obj) {
    std::vector<ubyte> bytes = json::to_binary(obj);
    return files::write_bytes(filename, (const char*)bytes.data(), bytes.size());
}

json::JObject* files::read_binary_json(std::filesystem::path file) {
    size_t size;
    std::unique_ptr<char[]> bytes (files::read_bytes(file, size));
    return json::from_binary((const ubyte*)bytes.get(), size);
}

json::JObject* files::read_json(std::filesystem::path filename) {
	std::string text = files::read_string(filename);
	try {
		auto obj = json::parse(filename.string(), text);
        return obj;
	} catch (const parsing_error& error) {
		LOG_ERROR("Could not to parse {}. What: {}", filename.string(), error.errorLog());
		Logger::getInstance().flush();
        throw std::runtime_error("Could not to parse " + filename.string());
    }
}

std::vector<std::string> files::read_list(std::filesystem::path filename) {
	std::ifstream file(filename);
	if (!file) {
		LOG_ERROR("Could not to open file {}", filename.u8string());
		Logger::getInstance().flush();
		throw std::runtime_error("Could not to open file " + filename.u8string());
	}
	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) {
		util::trim(line);
		if (line.length() == 0) continue;
		if (line[0] == '#') continue;
		lines.push_back(line);
	}
	return lines;
}
