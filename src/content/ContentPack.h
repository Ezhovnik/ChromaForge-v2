#pragma once

#include <string>
#include <vector>

#include <typedefs.h>
#include <content/content_fwd.h>
#include <io/io.h>

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
    io::path folder;
public:
    contentpack_error(
        std::string packId,
        io::path folder,
        const std::string& message
    );

    std::string getPackId() const;
    io::path getFolder() const;
};

struct ContentPack {
    std::string id = "none";
    std::string title = "untitled";
    std::string version = "0.0";
    std::string creator = "";
    std::string description = "no description";

    std::vector<DependencyPack> dependencies;

    io::path folder;
    std::string path;

    std::string source = "";

    io::path getContentFile() const;

    static inline const std::string PACKAGE_FILENAME = "package.json";
    static inline const std::string CONTENT_FILENAME = "content.json";
    static inline const io::path BLOCKS_FOLDER = "blocks";
    static inline const io::path ITEMS_FOLDER = "items";
    static inline const io::path ENTITIES_FOLDER = "entities";
    static inline const io::path GENERATORS_FOLDER = "generators";
    static const std::vector<std::string> RESERVED_NAMES;

    static bool is_pack(const io::path& folder);
    static ContentPack read(const std::string& path, const io::path& folder);

    static void scanFolder(
        const std::string& path,
        const io::path& folder,
        std::vector<ContentPack>& packs
    );

    static std::vector<std::string> worldPacksList(
        const io::path& folder
    );
    static io::path findPack(
        const EnginePaths* paths, 
        const io::path& worldDir, 
        const std::string& name
    );

    static ContentPack createBuiltin(const EnginePaths&);

    static inline io::path getFolderFor(ContentType type) {
        switch (type) {
            case ContentType::Block: return ContentPack::BLOCKS_FOLDER;
            case ContentType::Item: return ContentPack::ITEMS_FOLDER;
            case ContentType::Entity: return ContentPack::ENTITIES_FOLDER;
            case ContentType::Generator: return ContentPack::GENERATORS_FOLDER;
            case ContentType::None: return "";
            default: return "";
        }
    }
};

struct ContentPackStats {
    size_t totalBlocks;
    size_t totalItems;
    size_t totalEntities;

    inline bool hasSavingContent() const {
        return totalBlocks + totalItems + totalEntities > 0;
    }
};

struct WorldFuncsSet {
    bool onblockplaced;
    bool onblockreplaced;
    bool onblockbreaking;
    bool onblockbroken;
    bool onblockinteract;
    bool onplayerspark;
    bool onchunkpresent;
    bool onchunkremove;
    bool oninventoryopen;
    bool oninventoryclosed;
};

class ContentPackRuntime {
private:
    ContentPack info;
    ContentPackStats stats {};
    scriptenv env;
public:
    WorldFuncsSet worldfuncsset {};

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
