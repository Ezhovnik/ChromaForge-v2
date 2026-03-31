#ifndef CONTENT_CONTENT_BUILDER_H_
#define CONTENT_CONTENT_BUILDER_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "../items/Item.h"
#include "../voxels/Block.h"
#include "../content/Content.h"
#include "../content/ContentPack.h"

class ContentBuilder {
private:
    std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs;
    std::vector<std::string> blockIds;

    std::unordered_map<std::string, std::unique_ptr<Item>> itemDefs;
    std::vector<std::string> itemIds;

    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials;
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs;
public:
    ~ContentBuilder();

    void add(std::unique_ptr<ContentPackRuntime> pack);

    Block& createBlock(std::string id);
    Item& createItem(std::string id);
    BlockMaterial& createBlockMaterial(std::string id);

    void checkIdentifier(std::string id);
    ContentType checkContentType(std::string id);

    std::unique_ptr<Content> build();
};

#endif // CONTENT_CONTENT_BUILDER_H_
