#include "ContentLUT.h"

#include <memory>

#include "Content.h"
#include "../constants.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../voxels/Block.h"
#include "../data/dynamic.h"
#include "../items/Item.h"

ContentLUT::ContentLUT(const Content* content, size_t blocksCount, size_t itemsCount) {
    ContentIndices* indices = content->getIndices();
    for (size_t i = 0; i < blocksCount; ++i) {
        blocks.push_back(i);
    }

    for (size_t i = 0; i < indices->countBlockDefs(); ++i) {
        blockNames.push_back(indices->getBlockDef(i)->name);
    }

    for (size_t i = indices->countBlockDefs(); i < blocksCount; ++i) {
        blockNames.emplace_back("");
    }

    for (size_t i = 0; i < itemsCount; ++i) {
        items.push_back(i);
    }
    for (size_t i = 0; i < indices->countItemDefs(); ++i) {
        itemNames.push_back(indices->getItemDef(i)->name);
    }
    for (size_t i = indices->countItemDefs(); i < itemsCount; ++i) {
        itemNames.emplace_back("");
    }
}

// Создаёт таблицу перекодировки из JSON-файла.
std::shared_ptr<ContentLUT> ContentLUT::create(const std::filesystem::path& filename, const Content* content) {
    // Чтение и разбор JSON-файла
    auto root = files::read_json(filename);
    auto blocklist = root->list("blocks");
    auto itemlist = root->list("items");

    // Размер таблицы = максимум из длины массива и числа блоков в текущем контенте
    auto* indices = content->getIndices();
    size_t blocks_c = blocklist ? std::max(blocklist->size(), indices->countBlockDefs()) : indices->countBlockDefs();
    size_t items_c = itemlist ? std::max(itemlist->size(), indices->countItemDefs()) : indices->countItemDefs();    

    // Создаём временный объект LUT с начальным заполнением
    auto lut = std::make_shared<ContentLUT>(content, blocks_c, items_c);

    // Если в JSON есть массив блоков, обрабатываем его
    if (blocklist) {
        for (size_t i = 0; i < blocklist->size(); ++i) {
            std::string name = blocklist->str(i);
            Block* def = content->findBlock(name);

            // Блок найден – сохраняем его текущий идентификатор
            if (def) lut->setBlock(i, name, def->rt.id);
            // Блок отсутствует – помечаем как пустоту
            else lut->setBlock(i, name, BLOCK_VOID);   
        }
    }

    if (itemlist) {
        for (size_t i = 0; i < itemlist->size(); ++i) {
            std::string name = itemlist->str(i);
            Item* def = content->findItem(name);
            if (def) lut->setItem(i, name, def->rt.id);
            else lut->setItem(i, name, ITEM_VOID);   
        }
    }

    if (lut->hasContentReorder() || lut->hasMissingContent()) {
        return lut;
    } else {
        return nullptr;
    }
}

std::vector<ContentEntry> ContentLUT::getMissingContent() const {
    std::vector<ContentEntry> entries;
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i] == BLOCK_VOID) {
            auto& name = blockNames[i];
            entries.push_back(ContentEntry{ContentType::Block, name});
        }
    }
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i] == ITEM_VOID) {
            auto& name = itemNames[i];
            entries.push_back(ContentEntry{ContentType::Item, name});
        }
    }
    return entries;
}
