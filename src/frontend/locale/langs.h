#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <unordered_map>

struct ContentPack;

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

    /// Текущий активный язык.
    extern std::unique_ptr<Lang> current;

    /// Словарь информации о всех доступных локалях (заполняется из langs.json).
    extern std::unordered_map<std::string, LocaleInfo> locales_info;

    /**
     * @brief Загружает информацию о доступных локалях из файла langs.json.
     * @param resdir Путь к ресурсам игры.
     * @param[out] fallback Сюда будет записана локаль по умолчанию из файла (или FALLBACK_DEFAULT).
     */
    extern void loadLocalesInfo(
        const std::filesystem::path& resdir, 
        std::string& fallback
    );

    /**
     * @brief Загружает переводы для конкретной локали из указанных источников.
     * @param resdir Путь к ресурсам.
     * @param locale Запрашиваемая локаль.
     * @param packs Список контент-паков (модов).
     * @param lang Объект Lang, в который будут добавлены переводы.
     */
    extern void load(
        const std::filesystem::path& resdir,
        const std::string& locale, 
        const std::vector<ContentPack>& packs, 
        Lang& lang
    );

    /**
     * @brief Загружает переводы для указанной локали, используя fallback при необходимости.
     * @param resdir Путь к ресурсам.
     * @param locale Запрашиваемая локаль.
     * @param fallback Локаль по умолчанию (будет загружена первой).
     * @param packs Список контент-паков.
     *
     * Сначала загружается fallback-локаль, затем (если locale != fallback) — запрошенная локаль.
     * Результат сохраняется в current.
     */
    extern void load(
        const std::filesystem::path& resdir, 
        const std::string& locale, 
        const std::string& fallback, 
        const std::vector<ContentPack>& packs
    );

    /**
     * @brief Возвращает перевод по ключу из текущего языка.
     * @param key Ключ.
     * @return Перевод или сам ключ.
     */
    extern const std::wstring& get(const std::wstring& key);

    /**
     * @brief Возвращает перевод по ключу с учётом контекста.
     * @param key Ключ.
     * @param context Контекст.
     * @return Перевод.
     */
    extern const std::wstring& get(const std::wstring& key, const std::wstring& context);

    /**
     * @brief Настраивает систему локализации: загружает информацию о локалях,
     *        проверяет доступность запрошенной локали, загружает переводы.
     * @param resdir Путь к ресурсам.
     * @param locale Запрашиваемая локаль.
     * @param packs Список контент-паков.
     */
    extern void setup(
        const std::filesystem::path& resdir, 
        std::string locale, 
        const std::vector<ContentPack>& packs
    );
}
