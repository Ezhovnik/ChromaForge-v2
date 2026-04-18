#ifndef UTIL_BUFFER_POOL_H_
#define UTIL_BUFFER_POOL_H_

#include <queue>
#include <mutex>
#include <vector>
#include <memory>

#include "typedefs.h"

namespace util {
    template<class T>
    class BufferPool {
    private:
        std::vector<std::unique_ptr<T[]>> buffers;
        std::queue<T*> freeBuffers;
        std::mutex mutex;
        size_t bufferSize;
    public:
        BufferPool(size_t bufferSize) : bufferSize(bufferSize) {
        }

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
    };
}

#endif // UTIL_BUFFER_POOL_H_
