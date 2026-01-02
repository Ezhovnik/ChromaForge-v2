#include "files.h"

#include <fstream>
#include <iostream>
#include <filesystem>

// Записывает данные в бинарный файл в указанную позицию
bool write_binary_file_part(const std::string filename, const char* data, size_t offset, size_t size){
	std::ofstream output(filename, std::ios::out | std::ios::binary | std::ios::in);
	if (!output.is_open()) return false;

	output.seekp(offset);
	output.write(data, size);

	return true;
}

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

	char* data = new char[length];
	input.read(data, length);
	input.close();

	return data;
}

// Проверяет наличие директории. Если её нет, то создает
bool ensureDirectoryExists(const std::string directory) {
    std::filesystem::path dirPath(directory);

    if (!std::filesystem::exists(dirPath)) {
        if (std::filesystem::create_directories(dirPath)) {
            std::cout << "Directory '" << directory << "' created successfully." << std::endl;
            return true;
        } else {
            std::cerr << "Failed to create directory '" << directory << "'" << std::endl;
            return false;
        }
    } else if (!std::filesystem::is_directory(dirPath)) {
        std::cerr << "Path '" << directory << "' exists but is not a directory!" << std::endl;
        return false;
    }

    return true;
}

// Декомпрессия данных, сжатых алгоритмом RLE (Run-Length Encoding)
uint decompressRLE(const char* src, uint length, char* dst, uint targetLength){
	uint offset = 0;
	for (uint i = 0; i < length;){
		unsigned char counter = src[i++];
		char c = src[i++];
		for (uint j = 0; j <= counter; ++j){
			dst[offset++] = c;
		}
	}
	return offset;
}

// Вычисляет размер данных после сжатия RLE
uint calcRLE(const char* src, uint length) {
	uint offset = 0;
	uint counter = 1;
	char c = src[0];

	for (uint i = 0; i < length; ++i){
		char cnext = src[i];
		if (cnext != c || counter == 256){
			offset += 2;
			c = cnext;
			counter = 0;
		}
		counter++;
	}
	return offset + 2;
}

// Сжатие данных алгоритмом RLE (Run-Length Encoding)
uint compressRLE(const char* src, uint length, char* dst) {
	uint offset = 0;
	uint counter = 1;
	char c = src[0];
	for (uint i = 1; i < length; ++i){
		char cnext = src[i];
		if (cnext != c || counter == 256){
			dst[offset++] = counter - 1;
			dst[offset++] = c;
			c = cnext;
			counter = 0;
		}
		counter++;
	}
	dst[offset++] = counter - 1;
	dst[offset++] = c;
	return offset;
}
