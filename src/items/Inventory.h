#pragma once

#include <vector>
#include <memory>

#include <items/ItemStack.h>
#include <typedefs.h>
#include <interfaces/Serializable.h>

class ContentIndices;
class ContentReport;

class Inventory : Serializable {
    int64_t id;
    std::vector<ItemStack> slots;
public:
    Inventory() = default;
    Inventory(int64_t id, size_t size);
    Inventory(const Inventory& orig);

    ItemStack& getSlot(size_t index);
    size_t findEmptySlot(size_t begin=0, size_t end=-1) const;
    size_t findSlotByItem(
        itemid_t id, size_t begin = 0, size_t end = -1, size_t minCount = 1
    );

    void move(
        ItemStack& item,
        const ContentIndices& indices,
        size_t begin=0,
        size_t end=-1
    );

    void resize(uint newSize);

    void deserialize(const dv::value& src) override;
    dv::value serialize() const override;

    void convert(const ContentReport* report);
    static void convert(dv::value& data, const ContentReport* report);

    int64_t getId() const {
        return id;
    }

    size_t size() const {
        return slots.size();
    }

    void setId(int64_t id) {
        this->id = id;
    }

    bool isVirtual() const {
        return id < 0;
    }

    static constexpr size_t npos = -1;
};
