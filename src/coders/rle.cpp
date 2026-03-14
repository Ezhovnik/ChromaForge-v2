#include "rle.h"

/**
 * Декодирует данные, сжатые простым RLE.
 * Формат: последовательность пар (len, byte), где len — количество повторений минус 1.
 * Для каждой пары повторяем байт len+1 раз.
 */
size_t rle::decode(const ubyte* src, size_t srclen, ubyte* dst) {
	size_t offset = 0; // текущая позиция в выходном буфере
	for (size_t i = 0; i < srclen;) {
		ubyte len = src[i++]; // длина серии минус 1
		ubyte c = src[i++]; // длина серии минус 1
		// длина серии минус 1
		for (size_t j = 0; j <= len; ++j) {
			dst[offset++] = c;
		}
	}
	return offset; // общее количество распакованных байт
}

/**
 * Кодирует данные простым RLE.
 * Проходит по входным данным, подсчитывая длину последовательности одинаковых байт.
 * Когда встречается другой байт или счётчик достигает максимума (255), записывается пара (counter, текущий байт).
 * counter = длина серии минус 1.
 */
size_t rle::encode(const ubyte* src, size_t srclen, ubyte* dst) {
	if (srclen == 0) return 0;

	size_t offset = 0; // позиция в выходном буфере
	ubyte counter = 0; // счётчик повторений текущего символа
	ubyte c = src[0]; // первый символ
	for (size_t i = 1; i < srclen; ++i) {
		ubyte cnext = src[i];
		// Если следующий символ отличается или счётчик достиг максимума (255)
		if (cnext != c || counter == 255) {
			// Записываем накопленную серию
			dst[offset++] = counter;
			dst[offset++] = c;
			// Начинаем новую серию
			c = cnext;
			counter = 0;
		} else {
			// Увеличиваем счётчик для текущего символа
			counter++;
		}
	}
	// Записываем последнюю серию
	dst[offset++] = counter;
	dst[offset++] = c;
	return offset;
}

/**
 * Декодирует данные, сжатые расширенным RLE.
 * Читает длину серии: если первый байт имеет старший бит 0x80, то длина занимает два байта,
 * иначе один. Затем читается байт символа и повторяется len+1 раз.
 */
size_t extrle::decode(const ubyte* src, size_t srclen, ubyte* dst) {
	size_t offset = 0;
	for (size_t i = 0; i < srclen;) {
		uint len = src[i++]; // первый байт длины
		if (len & 0x80) { // если установлен старший бит — длина двухбайтовая
			len &= 0x7F;
			len |= ((uint)src[i++]) << 7; // добавляем второй байт (сдвиг на 7 бит)
		}
		ubyte c = src[i++]; // символ
		for (size_t j = 0; j <= len; ++j) {
			dst[offset++] = c;
		}
	}
	return offset;
}

/**
 * Кодирует данные расширенным RLE.
 * Работает аналогично простому RLE, но позволяет накапливать серии до max_sequence (0x7FFF).
 * При записи серии проверяет, помещается ли длина в один байт (len < 0x80) или требует двух.
 */
size_t extrle::encode(const ubyte* src, size_t srclen, ubyte* dst) {
	if (srclen == 0) return 0;

	size_t offset = 0;
	uint counter = 0; // счётчик повторений (до max_sequence)
	ubyte c = src[0];
	for (size_t i = 1; i < srclen; ++i) {
		ubyte cnext = src[i];
		if (cnext != c || counter == max_sequence) {
			// Если символ изменился или достигнут максимум длины серии
			if (counter >= 0x80) {
				dst[offset++] = 0x80 | (counter & 0x7F);
				dst[offset++] = counter >> 7;
			} else {
				dst[offset++] = counter;
			}
			dst[offset++] = c;
			c = cnext;
			counter = 0;
		} else {
			counter++;
		}
	}

	// Запись последней серии
	if (counter >= 0x80) {
		dst[offset++] = 0x80 | (counter & 0x7F);
		dst[offset++] = counter >> 7;
	} else {
		dst[offset++] = counter;
	}
	dst[offset++] = c;
	return offset;
}
