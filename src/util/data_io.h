#pragma once

#include <typedefs.h>

/**
 * @brief Функции для чтения и записи целых чисел в формате big-endian (сетевой порядок байт).
 *
 * Все функции работают с сырыми байтовыми буферами (массивами ubyte) и предполагают,
 * что вызывающий код гарантирует достаточный размер буфера для операций.
 * Смещение (offset) задаётся в байтах.
 */
namespace dataio {
    template <typename T>
    T swap(T value) {
        union{
            T value;
            ubyte bytes[sizeof(T)];
        } source, dest;

        source.value = value;

        for (size_t i = 0; i < sizeof(T); ++i) {
            dest.bytes[i] = source.bytes[sizeof(T) - i - 1];
        }
        return dest.value;
    }

    inline bool is_big_endian() {
        uint16_t num = 1;
        auto bytes = reinterpret_cast<uint8_t*>(&num);
        return bytes[1] == 1;
    }

    template <typename T>
    T be2h(T value){
        if (is_big_endian()) {
            return value;
        } else {
            return swap(value);
        }
    }

    template <typename T>
    T le2h(T value){
        if (is_big_endian()) {
            return swap(value);
        } else {
            return value;
        }
    }

    /**
     * @brief Читает 16-битное знаковое целое из буфера (big-endian).
     * @param src Указатель на буфер с данными.
     * @param offset Смещение в буфере (в байтах), откуда начинается чтение.
     * @return Прочитанное значение int16_t.
     */
    inline int16_t read_int16_big(const ubyte* src, size_t offset) {
        return (src[offset] << 8) | (src[offset + 1]);
    }

    /**
     * @brief Читает 32-битное знаковое целое из буфера (big-endian).
     * @param src Указатель на буфер.
     * @param offset Смещение.
     * @return Значение int32_t.
     */
    inline int32_t read_int32_big(const ubyte* src, size_t offset) {
        return (src[offset] << 24) | 
            (src[offset + 1] << 16) | 
            (src[offset + 2] << 8) | 
            (src[offset + 3]);
    }

    /**
     * @brief Читает 64-битное знаковое целое из буфера (big-endian).
     * @param src Указатель на буфер.
     * @param offset Смещение.
     * @return Значение int64_t.
     */
    inline int64_t read_int64_big(const ubyte* src, size_t offset) {
        return (int64_t(src[offset]) << 56) | 
            (int64_t(src[offset + 1]) << 48) | 
            (int64_t(src[offset + 2]) << 40) | 
            (int64_t(src[offset + 3]) << 32) |
            (int64_t(src[offset + 4]) << 24) | 
            (int64_t(src[offset + 5]) << 16) | 
            (int64_t(src[offset + 6]) << 8) | 
            (int64_t(src[offset + 7]));
    }

    /**
     * @brief Записывает 16-битное знаковое целое в буфер (big-endian).
     * @param value Значение для записи.
     * @param dest Указатель на буфер назначения.
     * @param offset Смещение в буфере, куда будут записаны байты.
     */
    inline void write_int16_big(int16_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 8 & 255);
        dest[offset + 1] = (char)(value >> 0 & 255);
    }

    /**
     * @brief Записывает 32-битное знаковое целое в буфер (big-endian).
     * @param value Значение для записи.
     * @param dest Указатель на буфер назначения.
     * @param offset Смещение в буфере, куда будут записаны байты.
     */
    inline void write_int32_big(int32_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 24 & 255);
        dest[offset + 1] = (char)(value >> 16 & 255);
        dest[offset + 2] = (char)(value >> 8 & 255);
        dest[offset + 3] = (char)(value >> 0 & 255);
    }

    /**
     * @brief Записывает 64-битное знаковое целое в буфер (big-endian).
     * @param value Значение для записи.
     * @param dest Указатель на буфер назначения
     * @param offset Смещение в буфере, куда будут записаны байты.
     */
    inline void write_int64_big(int64_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 56 & 255);
        dest[offset + 1] = (char)(value >> 48 & 255);
        dest[offset + 2] = (char)(value >> 40 & 255);
        dest[offset + 3] = (char)(value >> 32 & 255);

        dest[offset + 4] = (char)(value >> 24 & 255);
        dest[offset + 5] = (char)(value >> 16 & 255);
        dest[offset + 6] = (char)(value >> 8 & 255);
        dest[offset + 7] = (char)(value >> 0 & 255);
    }
}
