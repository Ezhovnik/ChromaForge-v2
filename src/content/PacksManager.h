#pragma once

#include <vector>
#include <unordered_map>

#include <content/ContentPack.h>
#include <io/io.h>

class PacksManager {
private:
    std::unordered_map<std::string, ContentPack> packs;
    std::vector<std::pair<std::string, io::path>> sources;
public:
    PacksManager();

    void setSources(std::vector<std::pair<std::string, io::path>> sources);

    void scan();

    std::vector<std::string> getAllNames() const;

    void exclude(const std::string& id);

    std::vector<ContentPack> getAll(const std::vector<std::string>& names) const;

    std::vector<std::string> assemble(const std::vector<std::string>& names) const;

    static std::vector<std::string> getNames(const std::vector<ContentPack>& packs);
};
