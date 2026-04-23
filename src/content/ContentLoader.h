#pragma once

#include <string>
#include <filesystem>
#include <memory>

#include <content/content_fwd.h>

class ContentBuilder;
struct ContentPack;
struct Item;
struct Entity;
class Block;
struct BlockMaterial;
struct ContentPackStats;
class ContentPackRuntime;
struct Generator;

namespace dynamic {
    class Map;
    class List;
}

class ContentLoader {
private:
    const ContentPack* pack;
    ContentPackRuntime* runtime;
    scriptenv env;
    ContentBuilder& builder;
    ContentPackStats* stats;

    void loadBlock(Block& def, const std::string& full, const std::string& name);
    void loadItem(Item& def, const std::string& full, const std::string& name);
    void loadEntity(Entity& def, const std::string& full, const std::string& name);
    void loadGenerator(Generator& def, const std::string& full, const std::string& name);

    static void loadCustomBlockModel(Block& def, dynamic::Map* primitives);
    static void loadBlockMaterial(BlockMaterial& def, const std::filesystem::path& file);
    static void loadBlock(Block& def, const std::string& name, const std::filesystem::path& file);
    static void loadItem(Item& def, const std::string& name, const std::filesystem::path& file);
    static void loadEntity(Entity& def, const std::string& name, const std::filesystem::path& file);
    void loadResources(ResourceType type, dynamic::List* list);
public:
    ContentLoader(ContentPack* pack, ContentBuilder& builder);

    bool fixPackIndices(
        const std::filesystem::path& folder,
        dynamic::Map* indicesRoot,
        const std::string& contentSection
    );
    void fixPackIndices();
    void load();
};
