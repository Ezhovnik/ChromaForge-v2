#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <stdlib.h>

#include "../typedefs.h"

extern bool write_binary_file(const std::string filename, const char* data, size_t size); // Записывает данные в бинарный файл (перезаписывает сущесвующий)
extern uint append_binary_file(const std::string filename, const char* data, size_t size); // Добавляет данные в конец бинарного файла
extern bool read_binary_file(const std::string filename, char* data, size_t size); // Читает данные из бинарного файла с начала
extern char* read_binary_file(std::string filename, size_t& length);

extern bool ensureDirectoryExists(const std::string directory); // Проверяет наличие директории. Если её нет, то создает
#endif // FILES_FILES_H_
