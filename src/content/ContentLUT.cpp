#include "ContentLUT.h"

#include <memory>

#include "Content.h"
#include "../constants.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../voxels/Block.h"
#include "../data/dynamic.h"

ContentLUT::ContentLUT(size_t blocksCount, const Content* content) {
    ContentIndices* indices = content->getIndices();
    for (size_t i = 0; i < blocksCount; ++i) {
        blocks.push_back(i);
    }

    for (size_t i = 0; i < indices->countBlockDefs(); ++i) {
        blockNames.push_back(indices->getBlockDef(i)->name);
    }

    for (size_t i = indices->countBlockDefs(); i < blocksCount; ++i) {
        blockNames.push_back("");
    }
}

// Создаёт таблицу перекодировки из JSON-файла.
ContentLUT* ContentLUT::create(const std::filesystem::path& filename, const Content* content) {
    // Чтение и разбор JSON-файла
    auto root = files::read_json(filename);
    auto blocklist = root->list("blocks");

    // Размер таблицы = максимум из длины массива и числа блоков в текущем контенте
    auto* indices = content->getIndices();
    size_t blocks_c = blocklist ? std::max(blocklist->size(), indices->countBlockDefs()) : indices->countBlockDefs();

    // Создаём временный объект LUT с начальным заполнением
    auto lut = std::make_unique<ContentLUT>(blocks_c, content);

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
    if (lut->hasContentReorder() || lut->hasMissingContent()) return lut.release();
    else return nullptr;
}
