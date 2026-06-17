#pragma once

#include <memory>
#include <unordered_map>
#include <utility>
#include <mutex>

#include <typedefs.h>
#include <delegates.h>

namespace util {
    class RunnablesList {
    private:
        int nextid = 1;
        std::unordered_map<int, runnable> runnables;
        std::mutex mutex;
    public:
        RunnablesList() = default;

        RunnablesList(RunnablesList&& o) {
            runnables = std::move(o.runnables);
            nextid = o.nextid;
        }

        void operator=(RunnablesList&& o) {
            runnables = std::move(o.runnables);
            nextid = o.nextid;
        }

        observer_handler add(runnable callback) {
            std::lock_guard lock(mutex);
            int id = nextid++;
            runnables[id] = std::move(callback);
            return observer_handler(new int(id), [this](int* id) {
                std::lock_guard lock(mutex);
                runnables.erase(*id);
                delete id;
            });
        }

        void notify() {
            std::lock_guard lock(mutex);
            for (auto& entry : runnables) {
                entry.second();
            }
        }
    };
}
