#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <items/Inventory.h>
#include <math/rand.h>

class Level;

using inventories_map = std::unordered_map<int64_t, std::shared_ptr<Inventory>>;

class Inventories {
private:
    Level& level;
    inventories_map map;
    PseudoRandom random;
public:
    Inventories(Level& level);
    ~Inventories();

    std::shared_ptr<Inventory> create(size_t size);

    std::shared_ptr<Inventory> createVirtual(size_t size);

    void store(const std::shared_ptr<Inventory>& inv);
    void remove(int64_t id);

    std::shared_ptr<Inventory> get(int64_t id);

    std::shared_ptr<Inventory> clone(int64_t id);

    const inventories_map& getMap() const;
};
