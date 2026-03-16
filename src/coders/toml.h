#ifndef CODERS_TOML_H_
#define CODERS_TOML_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "commons.h"

/**
 * @brief Пространство имён для работы с TOML-файлами.
 *
 * Содержит классы для описания структуры TOML-документа,
 * привязки полей к переменным и чтения/записи.
 */
namespace toml {
    /**
     * @brief Типы полей, поддерживаемые для привязки.
     */
    enum class fieldtype {
        ftbool, ///< Логическое значение (bool)
        ftint, ///< Знаковое целое (int)
        ftuint, ///< Беззнаковое целое (uint)
        ftfloat, ///< Число с плавающей точкой (float)
        ftdouble, ///< Число с плавающей точкой(double)
        ftstring, ///< Строка (std::string)
    };

    /**
     * @brief Описание одного поля в секции.
     *
     * Содержит тип и указатель на переменную, в которую будет прочитано значение.
     */
    struct Field {
        fieldtype type; ///< Тип поля
        void* ptr; ///< Указатель на целевую переменную
    };

    /**
     * @brief Секция TOML-документа (заголовок [имя]).
     *
     * Хранит привязанные поля и их порядок.
     */
    class Section {
        std::unordered_map<std::string, Field> fields; ///< Карта (имя -> поле)
        std::vector<std::string> keyOrder; ///< Порядок добавления ключей
        std::string name; ///< Имя секции

        /**
         * @brief Внутренний метод добавления поля в секцию.
         * @param name Имя ключа.
         * @param field Структура Field с типом и указателем.
         * @throw std::runtime_error если ключ с таким именем уже существует.
         */
        void add(std::string name, Field field);
    public:
        /**
         * @brief Конструктор секции.
         * @param name Имя секции (без квадратных скобок).
         */
        Section(std::string name);

        /**
         * @brief Привязать поле типа bool.
         * @param name Имя ключа в TOML.
         * @param ptr Указатель на переменную bool.
         */
        void add(std::string name, bool* ptr);

        /**
         * @brief Привязать поле типа int.
         * @param name Имя ключа.
         * @param ptr Указатель на переменную int.
         */
        void add(std::string name, int* ptr);

        /**
         * @brief Привязать поле типа uint.
         * @param name Имя ключа.
         * @param ptr Указатель на переменную uint.
         */
        void add(std::string name, uint* ptr);

        /**
         * @brief Привязать поле типа float.
         * @param name Имя ключа.
         * @param ptr Указатель на переменную float.
         */
        void add(std::string name, float* ptr);

        /**
         * @brief Привязать поле типа double.
         * @param name Имя ключа.
         * @param ptr Указатель на переменную double.
         */
        void add(std::string name, double* ptr);

        /**
         * @brief Привязать поле типа std::string.
         * @param name Имя ключа.
         * @param ptr Указатель на переменную std::string.
         */
        void add(std::string name, std::string* ptr);

        /**
         * @brief Получить описание поля по имени.
         * @param name Имя ключа.
         * @return Указатель на Field или nullptr, если ключ не найден.
         */
        const Field* field(const std::string& name) const;

        /**
         * @brief Установить значение поля (для записи в TOML) из double.
         * @param name Имя ключа.
         * @param value Значение.
         */
        void set(const std::string& name, double value);

        /**
         * @brief Установить значение поля из bool.
         * @param name Имя ключа.
         * @param value Значение.
         */
        void set(const std::string& name, bool value);

        /**
         * @brief Установить значение поля из строки.
         * @param name Имя ключа.
         * @param value Значение.
         */
        void set(const std::string& name, std::string value);

        /**
         * @brief Получить имя секции.
         * @return Имя.
         */
        const std::string& getName() const;

        /**
         * @brief Получить список имён ключей в порядке добавления.
         * @return Ссылка на вектор.
         */
        const std::vector<std::string>& keys() const;
    };

    /**
     * @brief Контейнер для всех секций TOML-документа.
     *
     * Управляет временем жизни секций и обеспечивает доступ по имени.
     */
    class Wrapper {
        std::unordered_map<std::string, Section*> sections; ///< Карта (имя секции -> объект)
        std::vector<std::string> keyOrder; ///< Порядок добавления секций
    public:
        ~Wrapper();

        /**
         * @brief Добавить новую секцию.
         * @param name Имя секции.
         * @return Ссылка на созданную секцию.
         * @throw std::runtime_error если секция с таким именем уже существует.
         */
        Section& add(std::string section);

        /**
         * @brief Получить секцию по имени.
         * @param name Имя секции.
         * @return Указатель на секцию или nullptr.
         */
        Section* section(std::string name);

        /**
         * @brief Сформировать TOML-строку из текущего состояния привязанных переменных.
         * @return Строка в формате TOML.
         */
        std::string write() const;
    };

    /**
     * @brief Парсер TOML-документа.
     *
     * Наследует BasicParser (из commons.h) и заполняет Wrapper данными из исходного текста.
     */
    class Reader : public BasicParser {
        Wrapper* wrapper; ///< Целевой контейнер для заполнения
        void skipWhitespace() override; ///< Переопределён для пропуска комментариев
        void readSection(Section* section); ///< Рекурсивное чтение секции
    public:
        /**
         * @brief Конструктор.
         * @param wrapper Указатель на Wrapper, который будет заполнен.
         * @param file Имя файла (для сообщений об ошибках).
         * @param source Содержимое TOML-файла.
         */
        Reader(Wrapper* wrapper, std::string file, std::string source);

        /**
         * @brief Запустить чтение.
         * @throw std::runtime_error при синтаксической ошибке.
         */
        void read();
    };
}

#endif // CODERS_TOML_H_
