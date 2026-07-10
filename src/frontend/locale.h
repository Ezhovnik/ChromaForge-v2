#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <io/fwd.h>

/**
 * @brief Пространство имён для системы локализации (переводов).
 *
 * Предоставляет функциональность загрузки языковых файлов,
 * хранения переводов и получения строк по ключам с учётом контекста.
 * Используется в основном для отображаемого текста,
 * поэтому использует в основном std::wstring.
 */
namespace langs {
    /// Расширение файлов с переводами.
    const char LANG_FILE_EXT[] = ".txt";
    /// Имя папки, содержащей файлы переводов.
    const char TEXTS_FOLDER[] = "texts";
    /// Язык по умолчанию (fallback), используемый если запрошенный не найден.
    const char FALLBACK_DEFAULT[] = "en_US";

    /**
     * @brief Класс, представляющий один язык.
     *
     * Хранит отображение «ключ(мод:контекст.ключ) → текст» для конкретной локали.
     */
    class Lang {
    private:
        std::string locale; ///< Идентификатор локали (например, "ru_RU")
        std::unordered_map<std::wstring, std::wstring> map; ///< Словарь переводов (ключ -> текст)
    public:
        /**
         * @brief Конструктор.
         * @param locale Идентификатор локали.
         */
        Lang(std::string locale);

        /**
         * @brief Возвращает перевод по ключу.
         * @param key Ключ.
         * @return Строка перевода или сам ключ, если перевод не найден.
         */
        const std::wstring& get(const std::wstring& key) const;

        /**
         * @brief Добавляет или заменяет перевод для указанного ключа.
         * @param key Ключ.
         * @param text Текст перевода.
         */
        void put(const std::wstring& key, const std::wstring& text);

        /**
         * @brief Возвращает идентификатор локали.
         * @return Строка с идентификатором.
         */
        const std::string& getId() const;
    };

    /**
     * @brief Информация о доступной локали для отображения в интерфейсе.
     */
    struct LocaleInfo {
        std::string locale; ///< Код локали (например, "ru_RU")
        std::string name; ///< Отображаемое название (например, "Русский")
    };

    std::string locale_by_envlocale(const std::string& envlocale);

    const std::string& get_current();
    const std::unordered_map<std::string, LocaleInfo>& get_locales_info();

    const std::wstring& get(const std::wstring& key);
    const std::wstring& get(
        const std::wstring& key, const std::wstring& context
    );

    void setup(
        std::string locale,
        const std::vector<io::path>& roots
    );
}
