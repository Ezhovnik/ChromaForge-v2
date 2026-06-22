#pragma once

#include <typedefs.h>
#include <constants.h>
#include <data/dv.h>

class ContentIndices;

class ItemStack {
    itemid_t item = ITEM_EMPTY;
    itemcount_t count = 0;
    dv::value fields = nullptr;
public:
    ItemStack() = default;

    ItemStack(itemid_t item, itemcount_t count, dv::value data=nullptr);

    void set(const ItemStack& item);
    void set(ItemStack&& item);
    void setCount(itemcount_t count);

    void setField(std::string_view name, dv::value value);
    dv::value* getField(const std::string& name) const;

    bool accepts(const ItemStack& item) const;
    void move(ItemStack& item, const ContentIndices& indices);

    void clear() {
        set(ItemStack(0, 0));
    }

    bool isEmpty() const {
        return item == ITEM_EMPTY;
    }

    itemid_t getItemId() const {
        return item;
    }

    itemcount_t getCount() const {
        return count;
    }

    const dv::value& getFields() const {
        return fields;
    }
};
