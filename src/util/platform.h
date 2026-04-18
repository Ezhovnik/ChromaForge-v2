#ifndef UTIL_PLATFORM_H_
#define UTIL_PLATFORM_H_

#include <string>
#include <filesystem>

/**
 * @brief Функции для работы с особенностями платформы (ОС).
 *
 * Содержит утилиты для настройки локали, кодировок и обнаружения системных параметров.
 */
namespace platform {
    /**
     * @brief Настраивает кодировку вывода для консоли.
     *
     * На Windows устанавливает UTF-8 для консольного вывода и буферизацию stdout.
     * Для других платформ функция пустая.
     */
    void configure_encoding();

    /**
     * @brief Определяет текущую системную локаль (язык/регион).
     *
     * @return Строка с идентификатором локали в формате "язык_СТРАНА" (например, "en_US").
     *         Если определение не удалось, возвращается значение DEFAULT_LOCALE.
     *
     * На Windows используется GetUserDefaultLocaleName, на Unix-подобных системах
     * анализируются переменные окружения LC_ALL и LANG.
     */
    std::string detect_locale();
}

#endif // UTIL_PLATFORM_H_
