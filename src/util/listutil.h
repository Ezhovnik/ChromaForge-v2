#ifndef UTIL_LISTUTIL_H_
#define UTIL_LISTUTIL_H_

#include <vector>

namespace util {
    /**
     * @brief Проверяет, содержится ли значение в векторе.
     * @tparam T Тип элементов вектора (должен поддерживать сравнение через operator==).
     * @param vec Вектор для поиска (передаётся по константной ссылке).
     * @param value Искомое значение.
     * @return true, если значение найдено; false в противном случае.
     */
    template<class T>
    bool contains(std::vector<T> vec, const T& value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }
}

#endif // UTIL_LISTUTIL_H_
