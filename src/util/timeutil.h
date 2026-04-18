#ifndef UTIL_TIMEUTIL_H_
#define UTIL_TIMEUTIL_H_

#include <chrono>

/**
 * @brief Пространство имён с утилитами для работы со временем.
 *
 * Содержит классы для измерения интервалов (Timer, ScopeLogTimer)
 * и функции для преобразования времени суток в долю дня и обратно.
 */
namespace timeutil {
    /**
     * @brief Простой таймер для измерения интервалов времени.
     *
     * Метод stop() возвращает количество микросекунд, прошедших с момента создания.
     */
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start;
    public:
        /**
         * @brief Конструктор, фиксирующий момент старта.
         */
        Timer();

        /**
         * @brief Останавливает таймер и возвращает прошедшее время.
         * @return Количество микросекунд с момента создания таймера.
         */
        int64_t stop();
    };

    /**
     * @brief Таймер, автоматически логирующий время своего существования.
     *
     * При создании запоминает идентификатор, а в деструкторе выводит в лог
     * время жизни (в микросекундах) с помощью LOG_DEBUG.
     */
    class ScopeLogTimer : public Timer{
    private:
        long long scopeid_; ///< Идентификатор блока
    public:
        /**
         * @brief Конструктор.
         * @param id Идентификатор, который будет выведен в лог при разрушении.
         */
        ScopeLogTimer(long long id);

        /**
         * @brief Деструктор, останавливающий таймер и выводящий время в лог.
         */
        ~ScopeLogTimer();
    };

    /**
     * @brief Преобразует время суток в долю дня (от 0 до 1).
     * @param hour Часы (0–23).
     * @param minute Минуты (0–59).
     * @param second Секунды (0–59).
     * @return Значение в диапазоне [0, 1), где 0 соответствует 00:00:00,
     *         0.5 — 12:00:00, и т.д.
     */
    inline constexpr float time_value(float hour, float minute, float second) {
        return (hour + (minute + second / 60.0f) / 60.0f) / 24.0f;
    }

    /**
     * @brief Преобразует долю дня обратно в часы, минуты, секунды.
     * @param value Доля дня (обычно от 0 до 1).
     * @param[out] hour Часть часов.
     * @param[out] minute Часть минут.
     * @param[out] second Часть секунд.
     */
    void from_value(float value, int& hour, int& minute, int& second);
}

#endif // UTIL_TIMEUTIL_H_
