#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

#include "../typedefs.h"

class ContentBuilder;
struct ContentPack;
class Item;
class Block;
struct BlockMaterial;

namespace dynamic {
    class Map;
}

class ContentLoader {
private:
    const ContentPack* pack;

    scriptenv env;

    void loadBlock(Block& block, const std::string& full, const std::string& name);
    void loadCustomBlockModel(Block& block, dynamic::Map* primitives);
    void loadItem(Item& item, const std::string& full, const std::string& name);
    void loadBlockMaterial(BlockMaterial& def, const std::filesystem::path& file);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(
        const std::filesystem::path& folder,
        dynamic::Map* indicesRoot, 
        const std::string& contentSection
    );
    void fixPackIndices();

    void loadBlock(
        Block& block,
        const std::string& name,
        const std::filesystem::path& file
    );
    void loadItem(
        Item& item,
        const std::string& name,
        const std::filesystem::path& file
    );
    void load(ContentBuilder& builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
