#ifndef CONTENT_CONTENT_LUT_H_
#define CONTENT_CONTENT_LUT_H_

#include <string>
#include <vector>
#include <filesystem>

#include "../typedefs.h"
#include "../constants.h"

class Content;

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

    // Флаг: изменился ли порядок блоков (индексы не совпадают)
    bool reorderContent = false;
    // Флаг: есть ли блоки, которые отсутствуют в текущем контенте
    bool missingContent = false;

public:
    ContentLUT(size_t blocksCount, const Content* content);

    // Возвращает имя блока для заданного старого индекса.
    inline const std::string& getBlockName(blockid_t index) const {return blockNames[index];}

    // Возвращает новый идентификатор блока для старого индекса.
    inline blockid_t getBlockId(blockid_t index) const {return blocks[index];}

    // Устанавливает соответствие для одного старого индекса.
    inline void setBlock(blockid_t index, std::string name, blockid_t id) {
        blocks[index] = id;
        blockNames[index] = name;
        if (id == BLOCK_VOID) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }

    // Статический метод для создания объекта ContentLUT из JSON-файла.
    // Читает файл indices.json, сопоставляет имена блоков с текущими определениями.
    static ContentLUT* create(const std::filesystem::path& filename, const Content* content);

    // Проверяет, требуется ли переупорядочивание блоков
    inline bool hasContentReorder() const {return reorderContent;}

    // Проверяет, есть ли отсутствующие в текущем контенте блоки
    inline bool hasMissingContent() const {return missingContent;}

    // Возвращает количество блоков в таблице (размер старого списка)
    inline size_t countBlocks() const {return blocks.size();}
};

#endif // CONTENT_CONTENT_LUT_H_
