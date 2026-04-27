#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <optional>

#include <typedefs.h>
#include <interfaces/Serializable.h>

namespace data {
    /**
     * @brief Перечисление поддерживаемых типов полей структуры.
     *
     * Определяет базовые скалярные типы, которые могут быть использованы
     * в описании полей структурной схемы.
     */
    enum class FieldType {
        I8 = 0,   ///< 8-битное знаковое целое (int8_t)
        I16,  ///< 16-битное знаковое целое (int16_t)
        I32,  ///< 32-битное знаковое целое (int32_t)
        I64,  ///< 64-битное знаковое целое (int64_t)
        F32,  ///< 32-битное число с плавающей точкой (float)
        F64,   ///< 64-битное число с плавающей точкой (double)
        CHAR,

        COUNT
    };

    inline std::string to_string(FieldType type) {
        const char* names[] = {
            "int8", "int16", "int32", "int64", "float32", "float64", "char" 
        };
        return names[static_cast<int>(type)];
    }
    FieldType FieldType_from_string(std::string_view name);

    enum class FieldIncapatibilityType {
        None = 0,
        Data_loss,
        Type_error,
        Missing,
    };

    struct FieldIncapatibility {
        std::string name;
        FieldIncapatibilityType type;
    };

    inline constexpr int sizeof_type(FieldType type) {
        const int sizes[] = {
            1, 2, 4, 8, 4, 8, 1
        };
        return sizes[static_cast<int>(type)];
    }

    class dataloss_error : public std::runtime_error {
    public:
        dataloss_error(const std::string& message) : std::runtime_error(message) {}
    };

    enum class FieldConvertStrategy {
        Reset = 0,
        Clamp
    };

    inline const char* to_string(FieldConvertStrategy strategy) {
        const char* names[] = {
            "reset", "clamp"
        };
        return names[static_cast<int>(strategy)];
    }
    FieldConvertStrategy FieldConvertStrategy_from_string(std::string_view name);

    /**
     * @brief Метаданные одного поля в структурной схеме.
     *
     * Содержит информацию о типе, имени и смещении поля относительно
     * начала данных структуры.
     */
    struct Field {
        FieldType type;     ///< Тип данных поля.
        std::string name;   ///< Имя поля (уникальное в пределах структуры).
        int elements;
        FieldConvertStrategy convertStrategy;
        int offset;         ///< Смещение поля в байтах от начала данных структуры.
        int size;

        bool operator==(const Field& o) const {
            return type == o.type && 
                name == o.name && 
                elements == o.elements &&
                convertStrategy == o.convertStrategy &&
                offset == o.offset &&
                size == o.size;
        }

        bool operator!=(const Field& o) const {
            return !operator==(o);
        }
    };

    /**
     * @brief Описание схемы (маппинга) структуры данных.
     *
     * Хранит упорядоченный список полей и обеспечивает быстрый доступ к
     * метаданным поля по его имени. Используется для интерпретации и доступа
     * к сырым данным, соответствующим описанной структуре.
     */
    class StructLayout : public Serializable{
        int totalSize;
        std::vector<Field> fields;                    ///< Упорядоченный список полей.
        std::unordered_map<std::string, int> indices; ///< Отображение имени поля на индекс в векторе fields.

        StructLayout(
            int totalSize,
            std::vector<Field> fields,
            std::unordered_map<std::string, int> indices
        ) : totalSize(totalSize), 
            fields(std::move(fields)),
            indices(std::move(indices))
        {}
    public:
        StructLayout() : StructLayout(0, {}, {}) {}

        bool operator==(const StructLayout& o) const {
            return fields == o.fields;
        }
        bool operator!=(const StructLayout& o) const {
            return !operator==(o);
        }

        /**
         * @brief Получение метаданных поля по его имени.
         *
         * Выполняет поиск поля с заданным именем в карте индексов.
         *
         * @param name Имя искомого поля.
         * @return Указатель на константный объект Field, если поле найдено;
         *         @c nullptr в противном случае.
         */
        [[nodiscard]]
        const Field* getField(const std::string& name) const {
            auto found = indices.find(name);
            if (found == indices.end()) {
                return nullptr;
            }
            return &fields.at(found->second);
        }

        const Field& requreField(const std::string& name) const;

        [[nodiscard]]
        integer_t getInteger(const ubyte* src, const std::string& name, int index=0) const;

        [[nodiscard]]
        number_t getNumber(const ubyte* src, const std::string& name, int index=0) const;

        [[nodiscard]]
        std::string_view getChars(const ubyte* src, const std::string& name) const;

        void setInteger(ubyte* dst, integer_t value, const std::string& name, int index=0) const;
        void setNumber(ubyte* dst, number_t value, const std::string& name, int index=0) const;
        size_t setAscii(ubyte* dst, std::string_view value, const std::string& name) const;

        size_t setUnicode(ubyte* dst, std::string_view value, const std::string& name) const;

        [[nodiscard]] size_t size() const {
            return totalSize;
        }

        void convert(
            const StructLayout& srcLayout, 
            const ubyte* src, 
            ubyte* dst,
            bool allowDataLoss
        ) const;

        std::vector<FieldIncapatibility> checkCompatibility(
            const StructLayout& dstLayout
        );

        [[nodiscard]]
        static StructLayout create(const std::vector<Field>& fields);

        std::unique_ptr<dynamic::Map> serialize() const override;
        void deserialize(dynamic::Map* src) override;
    };
} // namespace data
