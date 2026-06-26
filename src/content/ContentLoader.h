#pragma once

#include <string>
#include <memory>

#include <content/content_fwd.h>
#include <data/dv.h>
#include <io/io.h>

class ContentBuilder;
struct ContentPack;
struct Item;
struct Entity;
class Block;
struct BlockMaterial;
struct ContentPackStats;
class ContentPackRuntime;
struct Generator;
class ResPaths;
class Content;

class ContentLoader {
private:
    const ContentPack* pack;
    ContentPackRuntime* runtime;
    scriptenv env;
    ContentBuilder& builder;
    ContentPackStats* stats;
    const ResPaths& paths;

    void loadBlock(Block& def, const std::string& full, const std::string& name);
    void loadItem(Item& def, const std::string& full, const std::string& name);
    void loadEntity(Entity& def, const std::string& full, const std::string& name);
    void loadGenerator(Generator& def, const std::string& full, const std::string& name);

    static void loadBlockMaterial(
        BlockMaterial& def, const io::path& file
    );
    void loadBlock(
        Block& def, const std::string& name, const io::path& file
    );
    void loadItem(
        Item& def, const std::string& name, const io::path& file
    );
    void loadEntity(
        Entity& def, const std::string& name, const io::path& file
    );
    void loadResources(ResourceType type, const dv::value& list);
    void loadResourceAliases(ResourceType type, const dv::value& aliases);

    void loadContent(const dv::value& map);
public:
    ContentLoader(
        ContentPack* pack,
        ContentBuilder& builder,
        const ResPaths& paths
    );

    static std::vector<std::tuple<std::string, std::string>> scanContent(
        const ContentPack& pack, ContentType type
    );

    static bool fixPackIndices(
        const io::path& folder,
        dv::value& indicesRoot,
        const std::string& contentSection
    );
    void fixPackIndices();
    void load();

    static void loadScripts(Content& content);
    static void loadWorldScript(ContentPackRuntime& pack);
    static void reloadScript(const Content& content, Block& block);
    static void reloadScript(const Content& content, Item& item);
};
