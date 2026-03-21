#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

#include "../voxels/Block.h"

class ContentBuilder;
struct ContentPack;
class Item;

namespace dynamic {
    class Map;
}

class ContentLoader {
private:
    const ContentPack* pack;

    scriptenv env;

    void loadBlock(Block& block, std::string full, std::string name);
    void loadCustomBlockModel(Block& block, dynamic::Map* primitives);

    void loadItem(Item& item, std::string full, std::string name);
    BlockMaterial loadBlockMaterial(std::filesystem::path file, std::string full);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(std::filesystem::path folder, dynamic::Map* indicesRoot, std::string contentSection);
    void fixPackIndices();

    void loadBlock(Block& block, std::string name, std::filesystem::path file);
    void loadItem(Item& item, std::string name, std::filesystem::path file);
    void load(ContentBuilder& builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
