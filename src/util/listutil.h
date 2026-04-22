#pragma once

#include <vector>
#include <queue>
#include <algorithm>
#include <string>

namespace util {
    /**
     * @brief Проверяет, содержится ли значение в векторе.
     * @tparam T Тип элементов вектора (должен поддерживать сравнение через operator==).
     * @param vec Вектор для поиска.
     * @param value Искомое значение.
     * @return true, если значение найдено; false в противном случае.
    */
    template<class T>
    bool contains(const std::vector<T>& vec, const T& value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }

    std::string to_string(const std::vector<std::string>& vec);
}
