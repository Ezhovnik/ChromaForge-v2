#include <coders/zip.h>

#include <math.h>
#include <memory>

#define ZLIB_CONST
#include <zlib.h>

#include <coders/byte_utils.h>

// Коэффициент для предварительного расчёта размера сжатого буфера.
// Оценка: оригинальный размер + 1% + 23 байта
inline constexpr float COMPRESS_BUFFER_FACTOR = 1.01f;
inline constexpr size_t COMPRESS_BUFFER_PADDING = 23;

std::vector<ubyte> zip::compress(const ubyte* src, size_t size) {
    // Вычисляем начальный размер буфера (с запасом)
    size_t buffer_size = COMPRESS_BUFFER_PADDING + size * COMPRESS_BUFFER_FACTOR;
    std::vector<ubyte> buffer;
    buffer.resize(buffer_size);

    // Инициализация потока zlib для сжатия
    z_stream defstream {};
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = size; // сколько байт входных данных
    defstream.next_in = src; // указатель на входные данные
    defstream.avail_out = buffer_size; // размер выходного буфера
    defstream.next_out = buffer.data(); // указатель на выходной буфер

    // Инициализация сжатия в формате gzip (16 + MAX_WBITS включает генерацию заголовка gzip)
    // Используем стратегию по умолчанию и уровень сжатия по умолчанию
    deflateInit2(&defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    // Выполняем сжатие с завершением потока (Z_FINISH)
    deflate(&defstream, Z_FINISH);
    // Освобождаем ресурсы
    deflateEnd(&defstream);

    // Фактический размер сжатых данных: разность между конечным указателем и началом буфера
    size_t compressed_size = defstream.next_out - buffer.data();
    buffer.resize(compressed_size); // Обрезаем буфер до реального размера
    return buffer;
}

std::vector<ubyte> zip::decompress(const ubyte* src, size_t size) {
    // Предполагается, что в последних 4 байтах сжатых данных хранится исходный размер
    size_t decompressed_size = *reinterpret_cast<const uint32_t*>(src + size - 4);
    std::vector<ubyte> buffer;
    buffer.resize(decompressed_size);

    // Инициализация потока для распаковки
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = size; // размер входных (сжатых) данных
    infstream.next_in = src; // указатель на сжатые данные
    infstream.avail_out = decompressed_size; // размер выходного буфера
    infstream.next_out = buffer.data(); // указатель на выходной буфер

    // Инициализация распаковки gzip (те же флаги, что и при сжатии)
    inflateInit2(&infstream, 16 + MAX_WBITS);
    // Выполняем распаковку
    inflate(&infstream, Z_NO_FLUSH);
    // Завершаем работу
    inflateEnd(&infstream);

    return buffer;
}
