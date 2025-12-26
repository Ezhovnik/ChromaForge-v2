#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <stdlib.h>

extern bool write_binary_file(std::string filename, const char* data, size_t size); // Запись в бинарный файл
extern bool read_binary_file(std::string filename, char* data, size_t size); // Чтение из бинарного файла

#endif // FILES_FILES_H_
