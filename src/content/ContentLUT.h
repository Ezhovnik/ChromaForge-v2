#ifndef CONTENT_CONTENT_LUT_H_
#define CONTENT_CONTENT_LUT_H_

#include <string>
#include <vector>
#include <filesystem>
#include <utility>

#include "typedefs.h"
#include "constants.h"
#include "Content.h"
#include "data/dynamic.h"

struct ContentEntry {
    ContentType type;
    std::string name;
};

template<typename T, class U>
class ContentUnitLUT {
private:
    std::vector<T> indices;
    std::vector<std::string> names;
    bool missingContent = false;
    bool reorderContent = false;
    T missingValue;
    ContentType type;
public:
    ContentUnitLUT(size_t count, const ContentUnitIndices<U>& unitIndices, T missingValue, ContentType type) : missingValue(missingValue), type(type) {
        for (size_t i = 0; i < count; ++i) {
            indices.push_back(i);
        }
        for (auto unit : unitIndices.getIterable()) {
            names.push_back(unit->name);
        }
        for (size_t i = unitIndices.count(); i < count; ++i) {
            names.emplace_back("");
        }
    }

    void setup(dynamic::List* list, const ContentUnitDefs<U>& defs) {
        if (list) {
            for (size_t i = 0; i < list->size(); ++i) {
                std::string name = list->str(i);
                if (auto def = defs.find(name)) {
                    set(i, name, def->rt.id);
                } else {
                    set(i, name, missingValue);   
                }
            }
        }
    }

    void getMissingContent(std::vector<ContentEntry>& entries) const {
        for (size_t i = 0; i < count(); ++i) {
            if (indices[i] == missingValue) {
                auto& name = names[i];
                entries.push_back(ContentEntry{type, name});
            }
        }
    }

    inline const std::string& getName(T index) const {
        return names[index];
    }

    inline T getId(T index) const {
        return indices[index];
    }

    inline void set(T index, std::string name, T id) {
        indices[index] = id;
        names[index] = std::move(name);
        if (id == missingValue) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }

    inline size_t count() const {
        return indices.size();
    }

    inline bool hasContentReorder() const {
        return reorderContent;
    }

    inline bool hasMissingContent() const {
        return missingContent;
    }
};

class ContentLUT {
public:
    ContentUnitLUT<blockid_t, Block> blocks;
    ContentUnitLUT<itemid_t, Item> items;

    ContentLUT(const ContentIndices* indices, size_t blocks, size_t items);

    static std::shared_ptr<ContentLUT> create(
        const std::filesystem::path& filename, 
        const Content* content
    );

    inline bool hasContentReorder() const {
        return blocks.hasContentReorder() || items.hasContentReorder();
    }

    inline bool hasMissingContent() const {
        return blocks.hasMissingContent() || items.hasMissingContent();
    }

    std::vector<ContentEntry> getMissingContent() const;
};

#endif // CONTENT_CONTENT_LUT_H_
