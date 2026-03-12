#include "files.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>

#include "../logger/Logger.h"
#include "../coders/json.h"
#include "../util/stringutil.h"
#include "../data/dynamic.h"
#include "../coders/zip.h"

files::rafile::rafile(std::filesystem::path filename) : file(filename, std::ios::binary | std::ios::ate) {
    if (!file) {
		LOG_ERROR("Could not to open file '{}'", filename.string());
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
bool files::write_bytes(const std::filesystem::path filename, const ubyte* data, size_t size) {
	std::ofstream output(filename, std::ios::binary);
	if (!output.is_open()) return false;

	output.write((const char*)data, size);
	output.close();

	return true;
}

// Добавляет данные в конец бинарного файла
uint files::append_bytes(const std::filesystem::path filename, const ubyte* data, size_t size) {
	std::ofstream output(filename, std::ios::binary | std::ios::app);
	if (!output.is_open()) return 0;

	uint position = output.tellp();
	output.write((const char*)data, size);
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

ubyte* files::read_bytes(std::filesystem::path filename, size_t& length) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return nullptr;
	input.seekg(0, std::ios_base::end);
	length = input.tellg();
	input.seekg(0, std::ios_base::beg);

	std::unique_ptr<char> data(new char[length]);
	input.read(data.get(), length);
	input.close();
	return (ubyte*)data.release();
}

std::string files::read_string(std::filesystem::path filename) {
	size_t size;
	std::unique_ptr<ubyte[]> bytes (read_bytes(filename, size));
	if (bytes == nullptr) {
		LOG_ERROR("Could not to load file '{}'", filename.string());
		throw std::runtime_error("Could not to load file '" + filename.string() + "'");
	}
	return std::string((const char*)bytes.get(), size);
}

bool files::write_string(std::filesystem::path filename, const std::string content) {
	std::ofstream file(filename);
	if (!file) return false;

	file << content;
	return true;
}

bool files::write_json(std::filesystem::path filename, const dynamic::Map* obj, bool nice) {
    return files::write_string(filename, json::stringify(obj, nice, "  "));
}

bool files::write_binary_json(std::filesystem::path filename, const dynamic::Map* obj, bool compression) {
    auto bytes = json::to_binary(obj, compression);
    return files::write_bytes(filename, bytes.data(), bytes.size());
}

std::unique_ptr<dynamic::Map> files::read_binary_json(std::filesystem::path file) {
    size_t size;
    std::unique_ptr<ubyte[]> bytes (files::read_bytes(file, size));
    return std::unique_ptr<dynamic::Map>(json::from_binary(bytes.get(), size));
}

std::unique_ptr<dynamic::Map> files::read_json(std::filesystem::path filename) {
	std::string text = files::read_string(filename);
	try {
		auto obj = json::parse(filename.string(), text);
        return obj;
	} catch (const parsing_error& error) {
		LOG_ERROR("Could not to parse {}. What: {}", filename.string(), error.errorLog());
        throw std::runtime_error("Could not to parse " + filename.string());
    }
}

std::vector<std::string> files::read_list(std::filesystem::path filename) {
	std::ifstream file(filename);
	if (!file) {
		LOG_ERROR("Could not to open file {}", filename.u8string());
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
