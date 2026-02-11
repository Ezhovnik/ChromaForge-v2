#include "files.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>

#include "../logger/Logger.h"

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

	std::unique_ptr<char> data {new char[length]};
	input.read(data.get(), length);
	input.close();
	return data.release();
}

std::string files::read_string(std::filesystem::path filename) {
	size_t size;
	std::unique_ptr<char> chars (read_bytes(filename, size));
	return std::string(chars.get(), size);
}

bool files::write_string(std::filesystem::path filename, const std::string content) {
	std::ofstream file(filename);
	if (!file) return false;

	file << content;
	return true;
}
