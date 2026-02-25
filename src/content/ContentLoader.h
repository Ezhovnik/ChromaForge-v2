#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

class Block;
class ContentBuilder;
class ContentPack;

class ContentLoader {
private:
    const ContentPack* pack;
public:
    ContentLoader(ContentPack* pack);

    Block* loadBlock(std::string name, std::filesystem::path file);
    void load(ContentBuilder* builder);

    void fixPackIndices();
};

#endif // CONTENT_CONTENT_LOADER_H_
