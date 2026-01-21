#include "files.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <filesystem>

#include "../logger/Logger.h"

bool files::write_bytes(std::string filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary);
	if (!output.is_open()) return false;
	output.write(data, size);
	output.close();
	return true;
}

uint files::append_bytes(std::string filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary | std::ios::app);
	if (!output.is_open()) return 0;
	uint position = output.tellp();
	output.write(data, size);
	output.close();
	return position;
}

bool files::read(std::string filename, char* data, size_t size) {
	std::ifstream output(filename, std::ios::binary);
	if (!output.is_open()) return false;
	output.read(data, size);
	output.close();
	return true;
}

char* files::read_bytes(std::string filename, size_t& length) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return nullptr;
	input.seekg(0, std::ios_base::end);
	length = input.tellg();
	input.seekg(0, std::ios_base::beg);

	std::unique_ptr<char> data {new char[length]};
	input.read(data.get(), length);
	input.close();
	return data.release();
}

std::string files::read_string(std::string filename) {
	size_t size;
	std::unique_ptr<char> chars (read_bytes(filename, size));
	return std::string(chars.get(), size);
}

bool files::write_string(std::string filename, const std::string content) {
	std::ofstream file(filename);
	if (!file) return false;
	file << content;
	return true;
}

bool files::ensureDirectoryExists(const std::string directory) {
    std::filesystem::path dirPath(directory);

    if (!std::filesystem::exists(dirPath)) {
        if (std::filesystem::create_directories(dirPath)) {
            LOG_INFO("Directory '{}' created successfully", directory);
            return true;
        } else {
            LOG_ERROR("Failed to create directory '{}'", directory);
            return false;
        }
    } else if (!std::filesystem::is_directory(dirPath)) {
        LOG_ERROR("Path '{}' exists but is not a directory!", directory);
        return false;
    }

    return true;
}
