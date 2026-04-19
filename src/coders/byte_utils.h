#ifndef CODERS_BYTE_UTILS_H_
#define CODERS_BYTE_UTILS_H_

#include <string>
#include <vector>

#include <typedefs.h>

/**
 * @brief Строитель бинарных данных в формате little-endian.
 *
 * Позволяет последовательно записывать различные типы данных в вектор байт.
 * Все многобайтовые значения записываются в порядке little-endian (младший байт первым).
 */
class ByteBuilder {
private:
    std::vector<ubyte> buffer;
public:
    /**
     * @brief Записывает один байт (8‑битное беззнаковое целое).
     * @param b Значение байта.
     */
    void put(ubyte b);

    /**
     * @brief Записывает C-строку (включая завершающий нулевой байт '\00').
     * @param str Нуль-терминированная строка.
     */
    void putCStr(const char* str);

    /**
     * @brief Записывает знаковое 16‑битное целое.
     * @param val Значение.
     */
    void putInt16(int16_t val);

    /**
     * @brief Записывает знаковое 32‑битное целое.
     * @param val Значение.
     */
    void putInt32(int32_t val);

    /**
     * @brief Записывает знаковое 64‑битное целое.
     * @param val Значение.
     */
    void putInt64(int64_t val);
    
    /**
     * @brief Записывает 32‑битное число с плавающей точкой.
     * @param val Значение.
     */
    void putFloat32(float val);

    /**
     * @brief Записывает 64‑битное число с плавающей точкой.
     * @param val Значение.
     */
    void putFloat64(double val);   

    /**
     * @brief Записывает строку как длину (uint32) и затем байты строки.
     * @param s Строка.
     */
    void put(const std::string& s);

    /**
     * @brief Записывает последовательность байт без дополнительного заголовка.
     * @param arr Указатель на данные.
     * @param size Количество байт.
     */
    void put(const ubyte* arr, size_t size);

    /**
     * @brief Устанавливает байт по указанной позиции.
     * @param position Позиция в буфере.
     * @param val Значение.
     */
    void set(size_t position, ubyte val);

    /**
     * @brief Устанавливает 16‑битное целое по указанной позиции.
     * @param position Позиция (должна оставлять место для 2 байт).
     * @param val Значение.
     */
    void setInt16(size_t position, int16_t val);

    /**
     * @brief Устанавливает 32‑битное целое по указанной позиции.
     * @param position Позиция (должна оставлять место для 4 байт).
     * @param val Значение.
     */
    void setInt32(size_t position, int32_t val);

    /**
     * @brief Устанавливает 64‑битное целое по указанной позиции.
     * @param position Позиция (должна оставлять место для 8 байт).
     * @param val Значение.
     */
    void setInt64(size_t position, int64_t val);

    /**
     * @brief Возвращает текущий размер буфера.
     * @return Количество записанных байт.
     */
    inline size_t size() const {
        return buffer.size();
    }

    /**
     * @brief Возвращает указатель на данные буфера.
     * @return Константный указатель на внутренний массив.
     */
    inline const ubyte* data() const {
        return buffer.data();
    }

    /**
     * @brief Завершает построение и возвращает готовый вектор байт.
     * @return Копия внутреннего буфера.
     */
    std::vector<ubyte> build();
};

/**
 * @brief Читатель бинарных данных в формате little-endian.
 *
 * Позволяет последовательно читать значения из массива байт.
 * Все многобайтовые значения интерпретируются в порядке little-endian (младший байт первым).
 */
class ByteReader {
private:
    const ubyte* data; ///< Указатель на начало буфера.
    size_t size; ///< Общий размер буфера.
    size_t pos; ///< Текущая позиция чтения.
public:
    /**
     * @brief Конструктор от существующего буфера.
     * @param data Указатель на данные.
     * @param size Размер буфера в байтах.
     */
    ByteReader(const ubyte* data, size_t size);

    /**
     * @brief Конструктор, ожидающий, что первые 4 байта содержат размер остальных данных.
     * @param data Указатель на буфер, первые 4 байта интерпретируются как длина (uint32).
     *
     * @warning Предполагается, что данные имеют формат: [длина][...]. После чтения длины
     *          общий размер устанавливается равным длине + 4 (заголовок). Если данные
     *          не соответствуют этому формату, поведение не определено.
     */
    ByteReader(const ubyte* data);

    /**
     * @brief Проверяет наличие магической последовательности в текущей позиции.
     * @param data Ожидаемая строка-магия.
     * @param size Её длина.
     * @throw std::runtime_error Если данные не совпадают или выход за границы.
     */
    void checkMagic(const char* data, size_t size);

    /**
     * @brief Читает один байт и перемещает указатель.
     * @return Следующий байт.
     * @throw std::runtime_error При попытке чтения за концом буфера.
     */
    ubyte get();

    /**
     * @brief Возвращает текущий байт без перемещения указателя.
     * @return Байт в текущей позиции.
     * @throw std::runtime_error При пустом буфере.
     */
    ubyte peek();

    /**
     * @brief Читает знаковое 16‑битное целое.
     * @return Прочитанное значение.
     * @throw std::runtime_error При попытке чтения за концом буфера.
     */
    int16_t getInt16();

    /**
     * @brief Читает знаковое 32‑битное целое.
     * @return Прочитанное значение.
     * @throw std::runtime_error При попытке чтения за концом буфера.
     */
    int32_t getInt32();

    /**
     * @brief Читает знаковое 64‑битное целое (little-endian).
     * @return Прочитанное значение.
     * @throw std::runtime_error При попытке чтения за концом буфера.
     */
    int64_t getInt64();

    /**
     * @brief Читает 32‑битное число с плавающей точкой.
     * @return Прочитанное значение.
     */
    float getFloat32();

    /**
     * @brief Читает 64‑битное число с плавающей точкой.
     * @return Прочитанное значение.
     */
    double getFloat64();

    /**
     * @brief Читает C-строку (до завершающего нуля) и возвращает указатель на неё.
     * @return Указатель на строку внутри буфера. Строка не копируется!
     * @warning Указатель действителен, пока жив буфер.
     */
    const char* getCString();

    /**
     * @brief Читает строку в формате [длина][байты] и возвращает её как std::string.
     * @return Прочитанная строка.
     * @throw std::runtime_error При попытке чтения за концом буфера.
     */
    std::string getString();

    /**
     * @brief Проверяет, остались ли непрочитанные данные.
     * @return true, если позиция меньше размера.
     */
    bool hasNext() const;

    /**
     * @brief Возвращает указатель на текущую позицию в буфере.
     * @return Указатель data + pos.
     */
    const ubyte* pointer() const;

    /**
     * @brief Пропускает указанное количество байт.
     * @param n Число байт для пропуска.
     */
    void skip(size_t n);
};

#endif // CODERS_BYTE_UTILS_H_
