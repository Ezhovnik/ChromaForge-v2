#include "files.h"

#include <fstream>
#include <iostream>

// Запись в бинарный файл
bool write_binary_file(std::string filename, const char* data, size_t size) {
    std::ofstream output(filename, std::ios::binary);
    if (!output.is_open()) {
        return false;
    }

    output.write(data, size);
    output.close();

    return true;
}

// Чтение из бинарного файла
bool read_binary_file(std::string filename, char* data, size_t size) {
    std::ifstream input(filename, std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    input.read(data, size);
    input.close();

    return true;
}
