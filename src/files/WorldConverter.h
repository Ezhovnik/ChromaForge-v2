#ifndef FILES_WORLD_CONVERTER_H_
#define FILES_WORLD_CONVERTER_H_

#include <queue>
#include <filesystem>
#include <memory>

class Content;
class ContentLUT;
class WorldFiles;

class WorldConverter {
    WorldFiles* wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<std::filesystem::path> regions;
public:
    WorldConverter(std::filesystem::path folder, const Content* content, std::shared_ptr<ContentLUT> const lut);
    ~WorldConverter();

    bool hasNext() const;
    void convertNext();

    void write();
};

#endif // FILES_WORLD_CONVERTER_H_
