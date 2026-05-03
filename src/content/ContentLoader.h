#pragma once

#include <string>
#include <filesystem>
#include <memory>

#include <content/content_fwd.h>
#include <data/dv.h>

class ContentBuilder;
struct ContentPack;
struct Item;
struct Entity;
class Block;
struct BlockMaterial;
struct ContentPackStats;
class ContentPackRuntime;
struct Generator;

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

    static void loadCustomBlockModel(Block& def, const dv::value& primitives);
    static void loadBlockMaterial(BlockMaterial& def, const std::filesystem::path& file);
    void loadBlock(Block& def, const std::string& name, const std::filesystem::path& file);
    void loadItem(Item& def, const std::string& name, const std::filesystem::path& file);
    void loadEntity(Entity& def, const std::string& name, const std::filesystem::path& file);
    void loadResources(ResourceType type, const dv::value& list);

    void loadContent(const dv::value& map);
public:
    ContentLoader(ContentPack* pack, ContentBuilder& builder);

    static std::vector<std::string> scanContent(
        const ContentPack& pack, ContentType type
    );

    static bool fixPackIndices(
        const std::filesystem::path& folder,
        dv::value& indicesRoot,
        const std::string& contentSection
    );
    void fixPackIndices();
    void load();
};
