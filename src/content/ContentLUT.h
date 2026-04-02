#ifndef CONTENT_CONTENT_LUT_H_
#define CONTENT_CONTENT_LUT_H_

#include <string>
#include <vector>
#include <filesystem>
#include <utility>

#include "../typedefs.h"
#include "../constants.h"
#include "Content.h"

struct ContentEntry {
    ContentType type;
    std::string name;
};

/*
Класс для создания таблицы перекодировки индексов блоков.
Используется при загрузке мира, если набор блоков изменился
(добавлены, удалены или переупорядочены блоки).
Читает файл indices.json и строит отображение старых индексов на новые.
*/
class ContentLUT { // Content Look-UP Table
    // Вектор новых идентификаторов блоков, соответствующих старым индексам
    std::vector<blockid_t> blocks;
    // Имена блоков для каждого старого индекса (из indices.json)
    std::vector<std::string> blockNames;

    std::vector<itemid_t> items;
    std::vector<std::string> itemNames;

    // Флаг: изменился ли порядок блоков (индексы не совпадают)
    bool reorderContent = false;
    // Флаг: есть ли блоки, которые отсутствуют в текущем контенте
    bool missingContent = false;

public:
    ContentLUT(const Content* content, size_t blocks, size_t items);

    // Возвращает имя блока для заданного старого индекса.
    inline const std::string& getBlockName(blockid_t index) const {return blockNames[index];}

    // Возвращает новый идентификатор блока для старого индекса.
    inline blockid_t getBlockId(blockid_t index) const {return blocks[index];}

    // Устанавливает соответствие для одного старого индекса.
    inline void setBlock(blockid_t index, std::string name, blockid_t id) {
        blocks[index] = id;
        blockNames[index] = std::move(name);
        if (id == BLOCK_VOID) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }

    inline const std::string& getItemName(blockid_t index) const {return itemNames[index];}

    inline itemid_t getItemId(itemid_t index) const {return items[index];}

    inline void setItem(itemid_t index, std::string name, itemid_t id) {
        items[index] = id;
        itemNames[index] = std::move(name);
        if (id == ITEM_VOID) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }

    // Статический метод для создания объекта ContentLUT из JSON-файла.
    // Читает файл indices.json, сопоставляет имена блоков с текущими определениями.
    static std::shared_ptr<ContentLUT> create(const std::filesystem::path& filename, const Content* content);

    // Проверяет, требуется ли переупорядочивание блоков
    inline bool hasContentReorder() const {return reorderContent;}

    // Проверяет, есть ли отсутствующие в текущем контенте блоки
    inline bool hasMissingContent() const {return missingContent;}

    // Возвращает количество блоков в таблице (размер старого списка)
    inline size_t countBlocks() const {return blocks.size();}

    inline size_t countItems() const {return items.size();}

    std::vector<ContentEntry> getMissingContent() const;
};

#endif // CONTENT_CONTENT_LUT_H_
