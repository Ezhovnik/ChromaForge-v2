#ifndef GRAPHICS_RENDER_BLOCKS_RENDERER_H_
#define GRAPHICS_RENDER_BLOCKS_RENDERER_H_

#include <stdlib.h>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "../../math/UVRegion.h"
#include "../../voxels/voxel.h"
#include "../../typedefs.h"

class Content;
class Mesh;
class Block;
class Chunk;
class Chunks;
class VoxelsVolume;
class ChunksStorage;
class ContentGfxCache;
struct EngineSettings;

/**
 * @brief Класс для рендеринга блоков (вокселей) в чанках.
 *
 * Строит меш для одного чанка, учитывая видимость граней, освещение,
 * модели блоков и их повороты.
 */
class BlocksRenderer {
private:
	const Content* const content;
	std::unique_ptr<float[]> vertexBuffer;
	std::unique_ptr<int[]> indexBuffer;
	size_t vertexOffset;
	size_t indexOffset, indexSize;
	size_t capacity;

    int voxelBufferPadding = 2;
	bool overflow = false; ///< Флаг переполнения буфера

	const Chunk* chunk = nullptr;
	std::unique_ptr<VoxelsVolume> voxelsBuffer;

	const Block* const* blockDefsCache;
	const ContentGfxCache* const cache;
	const EngineSettings* settings;

    /**
     * @brief Добавляет вершину в буфер.
     * @param coord Координаты вершины.
     * @param u Текстурная координата U.
     * @param v Текстурная координата V.
     * @param light Цвет освещения (RGBA, каждый компонент 0..1).
     *
     * Упаковывает light в 32-битное число (4 байта) и сохраняет как float.
     */
	void vertex(const glm::vec3& coord, float u, float v, const glm::vec4& light);

    /**
     * @brief Добавляет шесть индексов для четырёх вершин (два треугольника).
     * @param a,b,c,d,e,f Индексы вершин (относительно текущего indexOffset).
     */
	void index(int a, int b, int c, int d, int e, int f);

    /**
     * @brief Вершинная функция с дополнительными осями для мягкого освещения (перегрузка).
     * @param coord Базовая координата.
     * @param u,v Текстурные координаты.
     * @param tint Цветовой оттенок.
     * @param axisX,axisY,axisZ Оси, определяющие ориентацию (нормализованные).
     */
	void vertex(
        const glm::vec3& coord, 
        float u, float v, 
        const glm::vec4& brightness,
		const glm::vec3& axisX,
        const glm::vec3& axisY,
		const glm::vec3& axisZ
    );

    /**
     * @brief Рисует грань с заданными размерами и осями (с освещением по углам).
     * @param coord Центр грани.
     * @param w,h,d Полуразмеры вдоль осей.
     * @param axisX,axisY,axisZ Оси ориентации.
     * @param region Текстурная область.
     * @param lights Массив из четырёх цветов освещения для углов.
     * @param tint Цветовой оттенок.
     */
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
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const glm::vec3& axisZ,
		const UVRegion& region,
        bool lights
    );

    /**
     * @brief Рисует четырёхугольную грань, заданную четырьмя точками в локальных координатах.
     * @param coord Базовая координата.
     * @param p1,p2,p3,p4 Локальные координаты углов (в диапазоне 0..1).
     * @param X,Y,Z Глобальные оси.
     * @param texreg Текстурная область.
     * @param lights Включить ли освещение.
     */
	void tetragonicFace(
        const glm::vec3& coord,
		const glm::vec3& p1, const glm::vec3& p2,
		const glm::vec3& p3, const glm::vec3& p4,
		const glm::vec3& X,
		const glm::vec3& Y,
		const glm::vec3& Z,
		const UVRegion& texreg,
		bool lights
    );

    /**
     * @brief Рендерит кубический блок (стандартная модель).
     * @param x,y,z Координаты блока в чанке.
     * @param faces Массив из 6 текстурных областей для каждой грани.
     * @param block Определение блока.
     * @param state Состояние блока.
     * @param lights Включить ли освещение.
     */
	void blockCube(
        int x, int y, int z, 
        const UVRegion(&faces)[6], 
        const Block* block, 
        blockstate state, 
        bool lights
    );

    /**
     * @brief Рендерит блок, представленный AABB.
     * @param coord Координаты блока.
     * @param faces Массив текстурных областей.
     * @param block Определение блока.
     * @param rotation Поворот.
     * @param lights Включить освещение.
     */
	void blockAABB(
        const glm::ivec3& coord,
        const UVRegion(&faces)[6], 
        const Block* block, 
        ubyte rotation,
        bool lights
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
     */
	void blockCustomModel(
        const glm::ivec3& icoord,
		const Block* block, 
        ubyte rotation,
		bool lights
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
	bool isOpen(int x, int y, int z, ubyte group) const;

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

    /**
     * @brief Основной метод рендеринга, проходящий по вокселям чанка.
     * @param voxels Массив вокселей чанка.
     */
	void render(const voxel* voxels);
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
        const Content* content, 
        const ContentGfxCache* cache, 
        const EngineSettings* settings
    );

	virtual ~BlocksRenderer();

    /**
     * @brief Строит данные для указанного чанка (без создания Mesh).
     * @param chunk Чанк.
     * @param chunks Хранилище чанков (для доступа к соседям).
     */
    void build(const Chunk* chunk, const ChunksStorage* chunks);

    /**
     * @brief Рендерит чанк и возвращает Mesh.
     * @param chunk Чанк.
     * @param chunks Хранилище чанков.
     * @return Указатель на новый Mesh.
     */
	std::shared_ptr<Mesh> render(const Chunk* chunk, const ChunksStorage* chunks);

    /**
     * @brief Создаёт Mesh из текущих буферов.
     * @return Указатель на новый Mesh.
     */
    std::shared_ptr<Mesh> createMesh();

    /**
     * @brief Возвращает буфер вокселей.
     */
	VoxelsVolume* getVoxelsBuffer() const;
};

#endif // GRAPHICS_RENDER_BLOCKS_RENDERER_H_
