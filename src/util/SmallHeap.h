#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <limits>

#include <util/Buffer.h>
#include <util/data_io.h>

namespace util {
    template<typename T>
    inline T read_int_le(const uint8_t* src, size_t offset=0) {
        return dataio::le2h(*(reinterpret_cast<const T*>(src) + offset));
    }

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
                auto nextIndex = read_int_le<Tindex>(data);
                data += sizeof(Tindex);
                auto nextSize = read_int_le<Tsize>(data);
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
                auto nextIndex = read_int_le<Tindex>(data);
                data += sizeof(Tindex);
                auto nextSize = read_int_le<Tsize>(data);
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
            *reinterpret_cast<Tindex*>(data) = dataio::h2le(index);
            data += sizeof(Tindex);
            *reinterpret_cast<Tsize*>(data) = dataio::h2le(size);
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
            return read_int_le<Tsize>(ptr, -1);
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

        inline bool operator==(const SmallHeap<Tindex, Tsize>& o) const {
            if (o.entriesCount != entriesCount) {
                return false;
            }
            return buffer == o.buffer;
        }

        util::Buffer<uint8_t> serialize() const {
            util::Buffer<uint8_t> out(sizeof(Tindex) + buffer.size());
            ubyte* dst = out.data();
            const ubyte* src = buffer.data();

            *reinterpret_cast<Tindex*>(dst) = dataio::h2le(entriesCount);
            dst += sizeof(Tindex);

            std::memcpy(dst, src, buffer.size());
            return out;
        }

        void deserialize(const ubyte* src, size_t size) {
            entriesCount = read_int_le<Tindex>(src);
            buffer.resize(size - sizeof(Tindex));
            std::memcpy(buffer.data(), src + sizeof(Tindex), buffer.size());
        }

        struct const_iterator {
        private:
            const std::vector<uint8_t>& buffer;
        public:
            Tindex index;
            size_t offset;

            const_iterator(
                const std::vector<uint8_t>& buffer,
                Tindex index,
                size_t offset
            ) : buffer(buffer), index(index), offset(offset) {}

            Tsize size() const {
                return read_int_le<Tsize>(buffer.data() + offset, -1);
            }

            bool operator!=(const const_iterator& o) const {
                return o.offset != offset;
            }

            const_iterator& operator++() {
                offset += size();
                if (offset == buffer.size()) {
                    return *this;
                }
                index = read_int_le<Tindex>(buffer.data() + offset);
                offset += sizeof(Tindex) + sizeof(Tsize);
                return *this;
            }

            const_iterator& operator*() {
                return *this;
            }

            const uint8_t* data() const {
                return buffer.data() + offset;
            }
        };

        const_iterator begin() const {
            if (buffer.empty()) {
                return end();
            }
            return const_iterator (
                buffer,
                read_int_le<Tindex>(buffer.data()),
                sizeof(Tindex) + sizeof(Tsize));
        }

        const_iterator end() const {
            return const_iterator (buffer, 0, buffer.size());
        }
    };
} // namespace util
