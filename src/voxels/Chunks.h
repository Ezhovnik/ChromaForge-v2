#pragma once

#include <stdlib.h>
#include <memory>
#include <vector>
#include <set>

#include <glm/glm.hpp>

#include <typedefs.h>
#include <voxels/voxel.h>
#include <util/AreaMap2D.h>

class Mesh;
class Chunk;
class WorldFiles;
class LevelEvents;
class Content;
class ContentIndices;
struct AABB;
class Block;
class Level;

// Класс для управления набором чанков в воксельном мире.
class Chunks{
private:
    Level* level;
	const ContentIndices* const contentIds;

    void eraseSegments(const Block& def, blockstate state, int x, int y, int z);
    void repairSegments(const Block& def, blockstate state, int x, int y, int z);
    void setRotationExtended(const Block& def, blockstate state, glm::ivec3 origin, uint8_t rotation);

    util::AreaMap2D<std::shared_ptr<Chunk>, int32_t> areaMap;
public:
    size_t visibleCount = 0;

    WorldFiles* worldFiles;

    Chunks(
        int32_t width, 
        int32_t depth, 
        int32_t areaOffsetX, 
        int32_t areaOffsetZ, 
        WorldFiles* worldFiles, 
        Level* level
    ); 
    ~Chunks() = default;

    bool checkReplaceability(const Block& def, blockstate state, glm::ivec3 coord, blockid_t ignore=0);

    bool putChunk(const std::shared_ptr<Chunk>& chunk);

    Chunk* getChunk(int32_t chunk_x, int32_t chunk_z); // Возвращает чанк по координатам чанка
    Chunk* getChunkByVoxel(int32_t x, int32_t y, int32_t z); // Получает чанк, содержащий воксель с заданными мировыми координатами

    voxel* getVoxel(int32_t x, int32_t y, int32_t z) const; // Возвращает воксель по мировым координатам
    inline voxel* getVoxel(glm::ivec3 pos) {
        return getVoxel(pos.x, pos.y, pos.z);
    }
    void setVoxel(int32_t x, int32_t y, int32_t z, blockid_t id, blockstate state); // Устанавливает идентификатор вокселя по мировым координатам

    light_t getLight(int32_t x, int32_t y, int32_t z);
	ubyte getLight(int32_t x, int32_t y, int32_t z, int channel);

    glm::ivec3 seekOrigin(glm::ivec3 pos, const Block& def, blockstate state);

    void setRotation(int32_t x, int32_t y, int32_t z, uint8_t rotation);

    voxel* rayCast( // Выполняет трассировку луча через воксельный мир.
        glm::vec3 start, // Начальная точка луча
        glm::vec3 dir, // Направление луча
        float maxDist, // Максимальная дистанция трассировки
        glm::vec3& end, // Точка попадания луча
        glm::ivec3& norm, // Нормаль поверхности в точке попадания
        glm::ivec3& iend, // Координаты вокселя в точке попадания
        std::set<blockid_t> filter = {}
    );
    glm::vec3 rayCastToObstacle(glm::vec3 start, glm::vec3 dir, float maxDist);

    const AABB* isObstacleAt(float x, float y, float z);
    bool isSolidBlock(int32_t x, int32_t y, int32_t z);
    bool isReplaceableBlock(int32_t x, int32_t y, int32_t z);
	bool isObstacleBlock(int32_t x, int32_t y, int32_t z);

    void setCenter(int32_t x, int32_t z);
    void resize(uint32_t newWidth, uint32_t newDepth);

    void saveAndClear();
    void save(Chunk* chunk);
    void saveAll();

    const std::vector<std::shared_ptr<Chunk>>& getChunks() const {
        return areaMap.getBuffer();
    }

    int32_t getWidth() const {
        return areaMap.getWidth();
    }

    int32_t getDepth() const {
        return areaMap.getDepth();
    }

    int32_t getOffsetX() const {
        return areaMap.getOffsetX();
    }

    int32_t getOffsetZ() const {
        return areaMap.getOffsetZ();
    }

    size_t getChunksCount() const {
        return areaMap.count();
    }

    size_t getVolume() const {
        return areaMap.area();
    }
};
