#pragma once

#include <memory>

#include <glm/glm.hpp>

#include <voxels/voxel.h>
#include <typedefs.h>
#include <voxels/Block.h>
#include <voxels/Chunk.h>
#include <voxels/VoxelsVolume.h>
#include <core_content_defs.h>
#include <graphics/render/commons.h>
#include <settings.h>
#include <math/rand.h>

template<typename VertexStructure> class Mesh;
class Content;
class Block;
class Chunk;
class VoxelsVolume;
class Chunks;
class ContentGfxCache;
struct UVRegion;

/**
 * @brief Класс для рендеринга блоков (вокселей) в чанках.
 *
 * Строит меш для одного чанка, учитывая видимость граней, освещение,
 * модели блоков и их повороты.
 */
class BlocksRenderer {
private:
	const Content& content;
	std::unique_ptr<ChunkVertex[]> vertexBuffer;
    std::unique_ptr<uint32_t[]> indexBuffer;
    std::unique_ptr<uint32_t[]> denseIndexBuffer;
    size_t vertexCount;
	size_t vertexOffset;
	size_t indexCount;
    size_t denseIndexCount;
	size_t capacity;

    int voxelBufferPadding = 2;
	bool overflow = false; ///< Флаг переполнения буфера
    bool cancelled = false;
    bool densePass = false;
    bool denseRender = false;

	const Chunk* chunk = nullptr;
	std::unique_ptr<VoxelsVolume> voxelsBuffer;

	const Block* const* blockDefsCache;
	const ContentGfxCache& cache;
    const EngineSettings& settings;

    util::PseudoRandom randomizer;

    SortingMeshData sortingMesh;

	void vertex(
        const glm::vec3& coord,
        float u, float v,
        const glm::vec4& light,
        const glm::vec3& normal,
        float emission
    );

    /**
     * @brief Добавляет шесть индексов для четырёх вершин (два треугольника).
     * @param a,b,c,d,e,f Индексы вершин (относительно текущего indexOffset).
     */
	void index(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f);

    /**
     * @brief Вершинная функция с дополнительными осями для мягкого освещения (перегрузка).
     * @param coord Базовая координата.
     * @param u,v Текстурные координаты.
     * @param tint Цветовой оттенок.
     * @param axisX,axisY,axisZ Оси, определяющие ориентацию (нормализованные).
     */
	void vertexAO(
        const glm::vec3& coord, 
        float u, float v, 
        const glm::vec4& brightness,
		const glm::vec3& axisX,
        const glm::vec3& axisY,
		const glm::vec3& axisZ
    );

	void face(
        const glm::vec3& coord, 
        float w, float h, float d,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
        const glm::vec3& axisZ,
		const UVRegion& region,
		const glm::vec4(&lights)[4],
		const glm::vec4& tint
    );

    /**
     * @brief Рисует грань с возможностью автоматического освещения.
     * @param coord Центр грани.
     * @param axisX,axisY,axisZ Оси (векторы направления).
     * @param region Текстурная область.
     * @param lights Если true, вычисляется освещение на основе направления к солнцу.
     */
	void face(
        const glm::vec3& coord,
        const glm::vec3& X,
        const glm::vec3& Y,
        const glm::vec3& Z,
        const UVRegion& region,
        glm::vec4 tint,
        bool lights
    );

    void faceAO(
        const glm::vec3& coord,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const glm::vec3& axisZ,
		const UVRegion& region,
        bool lights
    );

    /**
     * @brief Рендерит кубический блок (стандартная модель).
     * @param x,y,z Координаты блока в чанке.
     * @param faces Массив из 6 текстурных областей для каждой грани.
     * @param block Определение блока.
     * @param state Состояние блока.
     * @param lights Включить ли освещение.
     * @param ao
     */
	void blockCube(
        const glm::ivec3& coord, 
        const UVRegion(&faces)[6], 
        const Block& block, 
        blockstate state, 
        bool lights,
        bool ao
    );

    /**
     * @brief Рендерит блок, представленный AABB.
     * @param coord Координаты блока.
     * @param faces Массив текстурных областей.
     * @param block Определение блока.
     * @param rotation Поворот.
     * @param lights Включить освещение.
     * @param ao
     */
	void blockAABB(
        const glm::ivec3& coord,
        const UVRegion(&faces)[6], 
        const Block* block, 
        ubyte rotation,
        bool lights,
        bool ao
    );

    /**
     * @brief Рендерит X-образный спрайт (например, растения).
     * @param x,y,z Координаты блока.
     * @param size Размеры спрайта.
     * @param face1,face2 Текстурные области для двух пересекающихся плоскостей.
     * @param spread Случайный разброс позиции.
     */
	void blockXSprite(
        int x, int y, int z, 
        const glm::vec3& size, 
        const UVRegion& face1, 
        const UVRegion& face2, 
        float spread
    );

    /**
     * @brief Рендерит блок с кастомной моделью (набор AABB и дополнительных граней).
     * @param icoord Координаты блока.
     * @param block Определение блока.
     * @param rotation Поворот.
     * @param lights Включить освещение.
     * @param ao
     */
	void blockCustomModel(
        const glm::ivec3& icoord,
		const Block& block, 
        blockstate states,
		bool lights,
        bool ao
    );

    /**
     * @brief Проверяет, пропускает ли блок свет.
     * @param x,y,z Глобальные координаты.
     * @return true, если блок пропускает свет.
     */
	bool isOpenForLight(int x, int y, int z) const;

    /**
     * @brief Проверяет, открыта ли грань (соседний блок прозрачен или не той группы отрисовки).
     * @param x,y,z Глобальные координаты соседнего блока.
     * @param group Группа отрисовки текущего блока.
     * @return true, если грань должна быть отрисована.
     */
	inline bool isOpen(const glm::ivec3& pos, const Block& def, const Variant& variant) const {
        auto vox = voxelsBuffer->pickBlock(
            chunk->chunk_x * CHUNK_WIDTH + pos.x,
            pos.y,
            chunk->chunk_z * CHUNK_DEPTH + pos.z
        );
        if (vox.id == BLOCK_VOID) {
            return false;
        }
        const auto& block = *blockDefsCache[vox.id];
        const auto& blockVariant = block.getVariantByBits(vox.state.userbits);
        uint8_t otherDrawGroup = blockVariant.drawGroup;
        if ((otherDrawGroup && (otherDrawGroup != variant.drawGroup)) || !blockVariant.rt.solid) {
            return true;
        }
        if (densePass) {
            return variant.culling == CullingMode::Optional;
        } else if (variant.culling == CullingMode::Optional) {
            return false;
        }
        if (variant.culling == CullingMode::Disabled && vox.id == def.rt.id) {
            return true;
        }
        return vox.id == BLOCK_AIR;
    }

    /**
     * @brief Возвращает цвет освещения в точке.
     * @param x,y,z Координаты.
     * @return Цвет (RGBA, 0..1).
     */
	glm::vec4 pickLight(int x, int y, int z) const;

    /**
     * @brief Возвращает цвет освещения в точке.
     * @param coord Координаты.
     * @return Цвет (RGBA, 0..1).
     */
	glm::vec4 pickLight(const glm::ivec3& coord) const;

    /**
     * @brief Возвращает цвет освещения в точке (усреднение по соседям).
     * @param coord Координаты.
     * @param right,up Направления для билинейной интерполяции.
     * @return Усреднённый цвет.
     */
	glm::vec4 pickSoftLight(
        const glm::ivec3& coord, 
        const glm::ivec3& right, 
        const glm::ivec3& up
    ) const;

    /**
     * @brief Возвращает цвет освещения в точке (усреднение по соседям).
     * @param x,y,z Координаты.
     * @param right,up Направления для билинейной интерполяции.
     * @return Усреднённый цвет.
     */
	glm::vec4 pickSoftLight(
        float x, float y, float z, 
        const glm::ivec3& right, 
        const glm::ivec3& up
    ) const;

	void render(const voxel* voxels, const int beginEnds[256][2]);
    SortingMeshData renderTranslucent(const voxel* voxels, int beginEnds[256][2]);
public:
    /**
     * @brief Конструктор.
     * @param capacity Размер буферов (количество float в vertexBuffer и int в indexBuffer).
     * @param content Контент.
     * @param cache Кэш графики.
     * @param settings Настройки движка.
     */
	BlocksRenderer(
        size_t capacity, 
        const Content& content, 
        const ContentGfxCache& cache, 
        const EngineSettings& settings
    );

	virtual ~BlocksRenderer();

    /**
     * @brief Строит данные для указанного чанка (без создания Mesh).
     * @param chunk Чанк.
     * @param chunks Хранилище чанков (для доступа к соседям).
     */
    void build(const Chunk* chunk, const Chunks* chunks);

    /**
     * @brief Рендерит чанк и возвращает Mesh.
     * @param chunk Чанк.
     * @param chunks Хранилище чанков.
     * @return Указатель на новый Mesh.
     */
	ChunkMesh render(const Chunk* chunk, const Chunks* chunks);

    /**
     * @brief Создаёт Mesh из текущих буферов.
     * @return Указатель на новый Mesh.
     */
    ChunkMeshData createMesh();

    /**
     * @brief Возвращает буфер вокселей.
     */
	VoxelsVolume* getVoxelsBuffer() const;

    size_t getMemoryConsumption() const;

    bool isCancelled() const {
        return cancelled;
    }
};
