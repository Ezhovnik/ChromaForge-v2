#pragma once

#include <queue>
#include <mutex>
#include <vector>
#include <memory>

#include <typedefs.h>

namespace util {
    /**
     * @brief Пул буферов фиксированного размера для переиспользования памяти.
     *
     * Класс предоставляет потокобезопасный механизм получения и возврата
     * буферов типа T заданного размера. Буферы автоматически возвращаются
     * в пул при разрушении последней разделяемой ссылки std::shared_ptr.
     *
     * @tparam T Тип элементов буфера.
    */
    template<class T>
    class BufferPool {
    private:
        /** Владеющие указатели на все выделенные буферы. */
        std::vector<std::unique_ptr<T[]>> buffers;

        /** Очередь свободных буферов, готовых к выдаче. */
        std::queue<T*> freeBuffers;

        /** Мьютекс для синхронизации доступа к пулу. */
        std::mutex mutex;

        /** Размер каждого буфера в количестве элементов типа T. */
        size_t bufferSize;
    public:
        /**
         * @brief Конструктор пула буферов.
         * @param bufferSize Размер каждого буфера в элементах типа T.
         */
        BufferPool(size_t bufferSize) : bufferSize(bufferSize) {
        }

        /**
         * @brief Получить буфер из пула.
         *
         * Если свободных буферов нет, создаётся новый. Возвращается
         * std::shared_ptr с пользовательским удалителем, который
         * автоматически возвращает буфер обратно в пул.
         *
         * @return Разделяемый умный указатель на массив из bufferSize элементов типа T.
         */
        std::shared_ptr<T[]> get() {
            std::lock_guard lock(mutex);
            if (freeBuffers.empty()) {
                buffers.emplace_back(std::make_unique<T[]>(bufferSize));
                freeBuffers.push(buffers[buffers.size() - 1].get());
            }
            auto* buffer = freeBuffers.front();
            freeBuffers.pop();
            return std::shared_ptr<T[]>(buffer, [this](T* ptr) {
                std::lock_guard lock(mutex);
                freeBuffers.push(ptr);
            });
        }

        size_t getBufferSize() const {
            return bufferSize;
        }
    };
} // namespace util
