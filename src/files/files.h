#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <stdlib.h>

typedef unsigned int uint;

extern bool write_binary_file_part(const std::string filename, const char* data, size_t offset, size_t size); // Записывает данные в бинарный файл в указанную позицию 
extern bool write_binary_file(const std::string filename, const char* data, size_t size); // Записывает данные в бинарный файл (перезаписывает сущесвующий)
extern uint append_binary_file(const std::string filename, const char* data, size_t size); // Добавляет данные в конец бинарного файла
extern bool read_binary_file(const std::string filename, char* data, size_t size); // Читает данные из бинарного файла с начала
extern bool read_binary_file(const std::string filename, char* data, size_t offset, size_t size); // Читает данные из бинарного файла с указанной позиции
extern char* read_binary_file(std::string filename, size_t& length);

extern bool ensureDirectoryExists(const std::string directory); // Проверяет наличие директории. Если её нет, то создает

extern uint calcRLE(const char* src, uint length); // Декомпрессия данных, сжатых алгоритмом RLE (Run-Length Encoding)
extern uint compressRLE(const char* src, uint length, char* dst); // Вычисляет размер данных после сжатия RLE
extern uint decompressRLE(const char* src, uint length, char* dst, uint targetLength); // Сжатие данных алгоритмом RLE (Run-Length Encoding)

#endif // FILES_FILES_H_
