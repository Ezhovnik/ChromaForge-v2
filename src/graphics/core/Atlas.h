#ifndef GRAPHICS_CORE_ATLAS_H_
#define GRAPHICS_CORE_ATLAS_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <set>

#include "../../math/UVRegion.h"
#include "../../typedefs.h"

class ImageData;
class Texture;

class Atlas {
private:
    std::unique_ptr<Texture> texture;
    std::unique_ptr<ImageData> image;
    std::unordered_map<std::string, UVRegion> regions;
public:
    Atlas(
        std::unique_ptr<ImageData> image,
        std::unordered_map<std::string, UVRegion> regions,
        bool prepare
    );
    ~Atlas();

    void prepare();

    bool has(const std::string& name) const;
    const UVRegion& get(const std::string& name) const;

    Texture* getTexture() const;
    ImageData* getImage() const;
};


struct atlasentry {
    std::string name;
    std::shared_ptr<ImageData> image;
};

class AtlasBuilder {
private:
    std::vector<atlasentry> entries;
    std::set<std::string> names;
public:
    AtlasBuilder() {}
    void add(std::string name, std::unique_ptr<ImageData> image);
    bool has(const std::string& name) const;
    const std::set<std::string>& getNames() {return names;};

    Atlas* build(uint extrusion, bool prepare=true, uint maxResolution=0);
};

#endif // GRAPHICS_CORE_ATLAS_H_
