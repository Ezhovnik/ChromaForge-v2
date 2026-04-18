#ifndef UTIL_RUNNABLES_LIST_H_
#define UTIL_RUNNABLES_LIST_H_

#include <memory>
#include <unordered_map>
#include <utility>

#include "typedefs.h"
#include "delegates.h"

namespace util {
    class RunnablesList {
    private:
        int nextid = 1;
        std::unordered_map<int, runnable> runnables;
    public:
        observer_handler add(runnable callback) {
            int id = nextid++;
            runnables[id] = std::move(callback);
            return observer_handler(new int(id), [this](int* id) {
                runnables.erase(*id);
                delete id;
            });
        }

        void notify() {
            for (auto& entry : runnables) {
                entry.second();
            }
        }
    };
}

#endif // UTIL_RUNNABLES_LIST_H_
