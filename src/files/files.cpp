#include "files.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>

#include "../logger/Logger.h"

// Записывает данные в бинарный файл (перезаписывает сущесвующий)
bool write_binary_file(const std::string filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary);
	if (!output.is_open()) return false;

	output.write(data, size);
	output.close();

	return true;
}

// Добавляет данные в конец бинарного файла
uint append_binary_file(const std::string filename, const char* data, size_t size) {
	std::ofstream output(filename, std::ios::binary | std::ios::app);
	if (!output.is_open()) return 0;

	uint position = output.tellp();
	output.write(data, size);
	output.close();

	return position;
}

// Читает данные из бинарного файла с начала
bool read_binary_file(const std::string filename, char* data, size_t size) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return false;

	input.read(data, size);
	input.close();

	return true;
}

// Читает данные из бинарного файла с указанной позиции
bool read_binary_file(const std::string filename, char* data, size_t offset, size_t size) {
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()) return false;

	input.seekg(offset);
	input.read(data, size);
	input.close();

	return true;
}

char* read_binary_file(std::string filename, size_t& length) {
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

// Проверяет наличие директории. Если её нет, то создает
bool ensureDirectoryExists(const std::string directory) {
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

// Декомпрессия данных, сжатых алгоритмом RLE (Run-Length Encoding)
size_t decompressRLE(const ubyte* src, size_t length, ubyte* dst, size_t targetLength){
	size_t offset = 0;
	for (size_t i = 0; i < length;){
		ubyte counter = src[i++];
		ubyte c = src[i++];
		for (uint j = 0; j <= counter; ++j){
			dst[offset++] = c;
		}
	}
	return offset;
}

// Вычисляет размер данных после сжатия RLE
size_t calcRLE(const ubyte* src, size_t length) {
	size_t offset = 0;
	size_t counter = 0;
	ubyte c = src[0];

	for (size_t i = 0; i < length; ++i){
		ubyte cnext = src[i];
		if (cnext != c || counter == 255){
			offset += 2;
			c = cnext;
			counter = 0;
		} else {
            counter++;
        }
	}
	return offset + 2;
}

// Сжатие данных алгоритмом RLE (Run-Length Encoding)
size_t compressRLE(const ubyte* src, size_t length, ubyte* dst) {
    if (length == 0) return 0;
	size_t offset = 0;
	uint counter = 0;
	ubyte c = src[0];
	for (size_t i = 1; i < length; ++i){
		ubyte cnext = src[i];
		if (cnext != c || counter == 255){
			dst[offset++] = counter;
			dst[offset++] = c;
			c = cnext;
			counter = 0;
		} else {
            counter++;
        }
	}
	dst[offset++] = counter;
	dst[offset++] = c;
	return offset;
}
