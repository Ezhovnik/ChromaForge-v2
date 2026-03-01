#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

class Block;
class ContentBuilder;
class ContentPack;
class Item;
namespace json {
    class JObject;
}

class ContentLoader {
private:
    const ContentPack* pack;

    void loadBlock(Block* block, std::string full, std::string name);
    void loadCustomBlockModel(Block* block, json::JObject* primitives);

    void loadItem(Item* item, std::string full, std::string name);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(std::filesystem::path folder, json::JObject* indicesRoot, std::string contentSection);
    void fixPackIndices();

    void loadBlock(Block* block, std::string name, std::filesystem::path file);
    void loadItem(Item* item, std::string name, std::filesystem::path file);
    void load(ContentBuilder* builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
