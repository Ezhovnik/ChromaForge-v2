#pragma once

#include <vector>
#include <memory>

namespace util {
    class ObjectsKeeper {
    private:
        std::vector<std::shared_ptr<void>> ptrs;
    public:
        virtual ~ObjectsKeeper() {}

        virtual void keepAlive(std::shared_ptr<void> ptr) {
            ptrs.push_back(ptr);
        }
    };
}
