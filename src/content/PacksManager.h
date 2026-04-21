#ifndef CONTENT_PACKS_MANAGER_H_
#define CONTENT_PACKS_MANAGER_H_

#include <vector>
#include <filesystem>
#include <unordered_map>

#include <content/ContentPack.h>

class PacksManager {
private:
    std::unordered_map<std::string, ContentPack> packs;
    std::vector<std::filesystem::path> sources;
public:
    PacksManager();

    void setSources(std::vector<std::filesystem::path> sources);

    void scan();

    std::vector<std::string> getAllNames() const;

    void exclude(const std::string& id);

    std::vector<ContentPack> getAll(const std::vector<std::string>& names) const;

    std::vector<std::string> assembly(const std::vector<std::string>& names) const;

    static std::vector<std::string> getNames(const std::vector<ContentPack>& packs);
};

#endif // CONTENT_PACKS_MANAGER_H_
