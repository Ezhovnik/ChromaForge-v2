#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <limits>

namespace util {

/**
 * @brief Компактное хранилище данных, организованное как сортированный список
 *        индексированных блоков в едином буфере памяти.
 *
 * Класс SmallHeap управляет непрерывной областью памяти (std::vector<uint8_t>),
 * в которой последовательно расположены записи. Каждая запись состоит из:
 * - индекса типа Tindex,
 * - размера данных типа Tsize,
 * - собственно данных заданного размера.
 *
 * Записи всегда поддерживаются отсортированными по значению индекса. Класс
 * автоматически перестраивает буфер при выделении, освобождении или изменении
 * размера блока, сохраняя порядок и компактность данных.
 *
 * @tparam Tindex Тип, используемый для уникального индекса каждой записи.
 *                Должен поддерживать стандартные операции сравнения.
 * @tparam Tsize  Тип, используемый для хранения размера данных записи.
 *                Ожидается, что это целочисленный тип.
 */
template <typename Tindex, typename Tsize>
class SmallHeap {
    std::vector<uint8_t> buffer;   ///< Буфер, содержащий все записи.
    Tindex entriesCount;           ///< Текущее количество записей в буфере.
public:
    /**
     * @brief Конструктор по умолчанию.
     *
     * Создаёт пустой объект SmallHeap с нулевым количеством записей.
     */
    SmallHeap() : entriesCount(0) {}

    /**
     * @brief Поиск записи по индексу.
     *
     * Выполняет линейный поиск записи с заданным индексом среди всех существующих
     * записей. Так как записи отсортированы по индексу, поиск прекращается при
     * обнаружении индекса больше искомого.
     *
     * @param index Индекс искомой записи.
     * @return Указатель на начало данных записи, если запись найдена;
     *         @c nullptr в противном случае.
     */
    uint8_t* find(Tindex index) {
        auto data = buffer.data();
        for (size_t i = 0; i < entriesCount; i++) {
            auto nextIndex = *reinterpret_cast<Tindex*>(data);
            data += sizeof(Tindex);
            auto nextSize = *reinterpret_cast<Tsize*>(data);
            data += sizeof(Tsize);
            if (nextIndex == index) {
                return data;
            } else if (nextIndex > index) {
                return nullptr;
            }
            data += nextSize;
        }
        return nullptr;
    }

    /**
     * @brief Освобождение записи по указателю на её данные.
     *
     * Удаляет запись, на данные которой указывает переданный указатель.
     * Буфер памяти сжимается, а счётчик записей уменьшается на единицу.
     * Если передан @c nullptr, метод ничего не делает.
     *
     * @param ptr Указатель на данные записи, которую необходимо удалить.
     *            Должен быть получен ранее вызовом find() или allocate().
     */
    void free(uint8_t* ptr) {
        if (ptr == nullptr) {
            return;
        }
        auto entrySize = sizeOf(ptr);
        auto begin =
            buffer.begin() +
            ((ptr - sizeof(Tsize) - sizeof(Tindex)) - buffer.data());
        buffer.erase(
            begin, begin + entrySize + sizeof(Tsize) + sizeof(Tindex)
        );
        entriesCount--;
    }

    /**
     * @brief Выделение или перевыделение памяти для записи с заданным индексом.
     *
     * Если запись с указанным индексом уже существует и её размер совпадает
     * с запрошенным, её данные обнуляются и возвращается указатель на них.
     * Если размер не совпадает, запись удаляется и создаётся заново.
     * Если запись отсутствует, она вставляется в правильную позицию согласно
     * сортировке по индексу.
     *
     * @param index Индекс записи.
     * @param size  Требуемый размер данных записи. Не должен быть равен нулю.
     * @return Указатель на начало данных выделенной записи.
     * @throw std::runtime_error Если @p size равен нулю.
     */
    uint8_t* allocate(Tindex index, size_t size) {
        const auto maxSize = std::numeric_limits<Tsize>::max();
        if (size > maxSize) {
            throw std::invalid_argument(
                "Requested " + std::to_string(size) + " bytes but limit is "+ std::to_string(maxSize)
            );
        }
        if (size == 0) {
            throw std::invalid_argument("Zero size");
        }
        ptrdiff_t offset = 0;
        if (auto found = find(index)) {
            auto entrySize = sizeOf(found);
            if (size == entrySize) {
                std::memset(found, 0, entrySize);
                return found;
            }
            this->free(found);
            return allocate(index, size);
        }
        for (size_t i = 0; i < entriesCount; i++) {
            auto data = buffer.data() + offset;
            auto nextIndex = *reinterpret_cast<Tindex*>(data);
            data += sizeof(Tindex);
            auto nextSize = *reinterpret_cast<Tsize*>(data);
            data += sizeof(Tsize);
            if (nextIndex > index) {
                break;
            }
            data += nextSize;
            offset = data - buffer.data();
        }
        buffer.insert(
            buffer.begin() + offset,
            size + sizeof(Tindex) + sizeof(Tsize),
            0
        );
        entriesCount++;

        auto data = buffer.data() + offset;
        *reinterpret_cast<Tindex*>(data) = index;
        data += sizeof(Tindex);
        *reinterpret_cast<Tsize*>(data) = size;
        return data + sizeof(Tsize);
    }

    /**
     * @brief Возвращает размер данных записи по указателю на её данные.
     *
     * @param ptr Указатель на данные записи (ранее полученный от find() или allocate()).
     *            Если равен @c nullptr, возвращается 0.
     * @return Размер данных записи в байтах.
     */
    Tsize sizeOf(uint8_t* ptr) const {
        if (ptr == nullptr) {
            return 0;
        }
        return *(reinterpret_cast<Tsize*>(ptr)-1);
    }

    /**
     * @brief Возвращает текущее количество записей в куче.
     *
     * @return Количество записей.
     */
    Tindex count() const {
        return entriesCount;
    }

    /**
     * @brief Возвращает общий размер занимаемого буфера в байтах.
     *
     * @return Размер буфера (включая служебную информацию и данные).
     */
    size_t size() const {
        return buffer.size();
    }
};

} // namespace util
