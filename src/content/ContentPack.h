#ifndef CONTENT_CONTENT_PACK_H_
#define CONTENT_CONTENT_PACK_H_

#include <string>
#include <filesystem>
#include <vector>

#include <typedefs.h>

class EnginePaths;

enum class DependencyLevel {
    Required,
    Optional,
    Weak,
};

struct DependencyPack {
    DependencyLevel level;
    std::string id;
};

class contentpack_error : public std::runtime_error {
    std::string packId;
    std::filesystem::path folder;
public:
    contentpack_error(
        std::string packId,
        std::filesystem::path folder,
        const std::string& message
    );

    std::string getPackId() const;
    std::filesystem::path getFolder() const;
};

struct ContentPack {
    std::string id = "none";
    std::string title = "untitled";
    std::string version = "0.0";
    std::string creator = "";
    std::string description = "no description";

    std::vector<DependencyPack> dependencies;

    std::filesystem::path folder;

    std::filesystem::path getContentFile() const;

    static inline const std::string PACKAGE_FILENAME = "package.json";
    static inline const std::string CONTENT_FILENAME = "content.json";
    static inline const std::filesystem::path BLOCKS_FOLDER = "blocks";
    static inline const std::filesystem::path ITEMS_FOLDER = "items";
    static inline const std::filesystem::path ENTITIES_FOLDER = "entities";
    static const std::vector<std::string> RESERVED_NAMES;

    static bool is_pack(const std::filesystem::path& folder);
    static ContentPack read(const std::filesystem::path& folder);

    static void scanFolder(
        const std::filesystem::path& folder,
        std::vector<ContentPack>& packs
    );

    static std::vector<std::string> worldPacksList(
        const std::filesystem::path& folder
    );
    static std::filesystem::path findPack(
        const EnginePaths* paths, 
        const std::filesystem::path& worldDir, 
        const std::string& name
    );
};

struct ContentPackStats {
    size_t totalBlocks;
    size_t totalItems;
    size_t totalEntities;

    inline bool hasSavingContent() const {
        return totalBlocks + totalItems + totalEntities > 0;
    }
};

struct world_funcs_set {
    bool onblockplaced : 1;
    bool onblockbroken : 1;
};

class ContentPackRuntime {
private:
    ContentPack info;
    ContentPackStats stats {};
    scriptenv env;
public:
    world_funcs_set worldfuncsset {};

    ContentPackRuntime(
        ContentPack info,
        scriptenv env
    );
    ~ContentPackRuntime();

    inline const ContentPackStats& getStats() const {
        return stats;
    }

    inline ContentPackStats& getStatsWriteable() {
        return stats;
    }

    inline const std::string& getId() {
        return info.id;
    }

    inline const ContentPack& getInfo() const {
        return info;
    }

    inline scriptenv getEnvironment() const {
        return env;
    }
};

#endif // CONTENT_CONTENT_PACK_H_
