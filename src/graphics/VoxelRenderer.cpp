#include "VoxelRenderer.h"

#include "Mesh.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../lighting/LightMap.h"
#include "../declarations.h"

// Целочисленное деление с округлением вниз для отрицательных чисел
inline int cdiv(int x, int a) {
    return (x < 0) ? (x / a - 1) : (x / a);
}

// Преобразует глобальную отрицательную координату в локальную внутри чанка
inline int local_neg(int x, int size) {
    return (x < 0) ? (size + x) : x;
}

// Преобразует глобальную координату в локальную внутри чанка
inline int local(int x, int size) {
    return (x >= size) ? (x - size) : local_neg(x, size);
}

// Получает указатель на чанк в окружающих чанках для заданных мировых координат 
const Chunk* get_chunk(int x, int y, int z, const Chunk** closes) {
    // Определяем, в каком из окружающих 27 чанков находится воксель
    int chunk_x = cdiv(x, CHUNK_WIDTH);
    int chunk_z = cdiv(z, CHUNK_DEPTH);
    
    int index = (chunk_z + 1) * 3 + (chunk_x + 1);
    return closes[index];
}

// Получает значение освещенности для вокселя
int get_light(int x, int y, int z, int channel, const Chunk** closes) {
    const Chunk* chunk = get_chunk(x, y, z, closes);
    if (!chunk) return 0;
    
    int local_x = local(x, CHUNK_WIDTH);
    int local_y = local(y, CHUNK_HEIGHT);
    int local_z = local(z, CHUNK_DEPTH);
    return chunk->light_map->get(local_x, local_y, local_z, channel);
}

// Получает данные вокселя по мировым координатам
voxel get_voxel(int x, int y, int z, const Chunk** closes) {
    const Chunk* chunk = get_chunk(x, y, z, closes);
    // if (!chunk) {
    //     voxel empty;
    //     empty.id = Blocks_id::AIR;
    //     return empty;
    // }
    
    int lx = local(x, CHUNK_WIDTH);
    int ly = local(y, CHUNK_HEIGHT);
    int lz = local(z, CHUNK_DEPTH);
    return chunk->voxels[(ly * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
}

// Проверяет, является ли соседний воксель блокирующим для отрисовки грани
bool is_blocked(int x, int y, int z, const Chunk** closes, ubyte group) {
    const Chunk* chunk = get_chunk(x, y, z, closes);
    if (!chunk) return true;
    
    voxel vox = get_voxel(x, y, z, closes);
    return Block::blocks[vox.id].get()->drawGroup == group;
}

// Настраивает текстурные координаты для грани блока
inline void setup_uv(int texture_id, float& u1, float& v1, float& u2, float& v2) {
    u1 = (texture_id % 16) * VoxelRenderer_Conts::UVSIZE;
    v1 = 1.0f - ((1 + texture_id / 16) * VoxelRenderer_Conts::UVSIZE);
    u2 = u1 + VoxelRenderer_Conts::UVSIZE;
    v2 = v1 + VoxelRenderer_Conts::UVSIZE;
}

// Добавляет вершину в буфер
void add_vertex(std::vector<float>& buffer, size_t& index, 
                float x, float y, float z,
                float u, float v,
                float r, float g, float b, float s) {
    buffer.push_back(x);
    buffer.push_back(y);
    buffer.push_back(z);

    buffer.push_back(u);
    buffer.push_back(v);

    buffer.push_back(r);
    buffer.push_back(g);
    buffer.push_back(b);
    buffer.push_back(s);

    index += VoxelRenderer_Conts::CHUNK_VERTEX_SIZE;
}

void _renderBlock(std::vector<float>& buffer, int x, int y, int z, const Chunk** closes, voxel vox, size_t& index) {
    float u1, v1, u2, v2;

    Block* block = Block::blocks[vox.id].get();
    ubyte group = block->drawGroup;

    int textureCopyFaces[6];
	int rotate = 0;

	for (int i = 0; i < 6; ++i){
		textureCopyFaces[i] = block->textureFaces[i];
	}

	if (block->rotatable){
		if (vox.states == 0x31){
			rotate = 1;
			textureCopyFaces[0] = block->textureFaces[2];
			textureCopyFaces[1] = block->textureFaces[3];
			textureCopyFaces[2] = block->textureFaces[0];
			textureCopyFaces[3] = block->textureFaces[1];
		} else if (vox.states == 0x32){
			rotate = 2;
		} else if (vox.states == 0x33){
			rotate = 3;
			textureCopyFaces[2] = block->textureFaces[4];
			textureCopyFaces[3] = block->textureFaces[5];
			textureCopyFaces[4] = block->textureFaces[2];
			textureCopyFaces[5] = block->textureFaces[3];
		}
	}

    // Левая грань (-X)
    if (!is_blocked(x - 1, y, z, closes, group)) {
        setup_uv(textureCopyFaces[0], u1, v1, u2, v2);

        float lr = get_light(x - 1, y, z, 0, closes) / 15.0f;
        float lg = get_light(x - 1, y, z, 1, closes) / 15.0f;
        float lb = get_light(x - 1, y, z, 2, closes) / 15.0f;
        float ls = get_light(x - 1, y, z, 3, closes) / 15.0f;
        
        float lr0 = (get_light(x - 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y, z - 1, 0, closes) + 
                    get_light(x - 1, y - 1, z, 0, closes)) / 75.0f;
        float lr1 = (get_light(x - 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y, z + 1, 0, closes) + 
                    get_light(x - 1, y + 1, z, 0, closes)) / 75.0f;
        float lr2 = (get_light(x - 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y, z - 1, 0, closes) + 
                    get_light(x - 1, y + 1, z, 0, closes)) / 75.0f;
        float lr3 = (get_light(x - 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y, z + 1, 0, closes) + 
                    get_light(x - 1, y - 1, z, 0, closes)) / 75.0f;
        
        float lg0 = (get_light(x - 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y, z - 1, 1, closes) + 
                    get_light(x - 1, y - 1, z, 1, closes)) / 75.0f;
        float lg1 = (get_light(x - 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y, z + 1, 1, closes) + 
                    get_light(x - 1, y + 1, z, 1, closes)) / 75.0f;
        float lg2 = (get_light(x - 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y, z - 1, 1, closes) + 
                    get_light(x - 1, y + 1, z, 1, closes)) / 75.0f;
        float lg3 = (get_light(x - 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y, z + 1, 1, closes) + 
                    get_light(x - 1, y - 1, z, 1, closes)) / 75.0f;
        
        float lb0 = (get_light(x - 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y, z - 1, 2, closes) + 
                    get_light(x - 1, y - 1, z, 2, closes)) / 75.0f;
        float lb1 = (get_light(x - 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y, z + 1, 2, closes) + 
                    get_light(x - 1, y + 1, z, 2, closes)) / 75.0f;
        float lb2 = (get_light(x - 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y, z - 1, 2, closes) + 
                    get_light(x - 1, y + 1, z, 2, closes)) / 75.0f;
        float lb3 = (get_light(x - 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y, z + 1, 2, closes) + 
                    get_light(x - 1, y - 1, z, 2, closes)) / 75.0f;
        
        float ls0 = (get_light(x - 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y, z - 1, 3, closes) + 
                    get_light(x - 1, y - 1, z, 3, closes)) / 75.0f;
        float ls1 = (get_light(x - 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y, z + 1, 3, closes) + 
                    get_light(x - 1, y + 1, z, 3, closes)) / 75.0f;
        float ls2 = (get_light(x - 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y, z - 1, 3, closes) + 
                    get_light(x - 1, y + 1, z, 3, closes)) / 75.0f;
        float ls3 = (get_light(x - 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y, z + 1, 3, closes) + 
                    get_light(x - 1, y - 1, z, 3, closes)) / 75.0f;

        if ((rotate == 0) || (rotate == 2)){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u2,v1, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		} else if (rotate == 1){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u2,v1, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		} else if (rotate == 3){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v1, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v2, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		}
    }
    // Правая грань (+X)
    if (!is_blocked(x + 1, y, z, closes, group)) {
        setup_uv(textureCopyFaces[1], u1, v1, u2, v2);

        float lr = get_light(x + 1, y, z, 0, closes) / 15.0f;
        float lg = get_light(x + 1, y, z, 1, closes) / 15.0f;
        float lb = get_light(x + 1, y, z, 2, closes) / 15.0f;
        float ls = get_light(x + 1, y, z, 3, closes) / 15.0f;
        
        float lr0 = (get_light(x + 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y, z - 1, 0, closes) + 
                    get_light(x + 1, y - 1, z, 0, closes)) / 75.0f;
        float lr1 = (get_light(x + 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y, z - 1, 0, closes) + 
                    get_light(x + 1, y + 1, z, 0, closes)) / 75.0f;
        float lr2 = (get_light(x + 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y, z + 1, 0, closes) + 
                    get_light(x + 1, y + 1, z, 0, closes)) / 75.0f;
        float lr3 = (get_light(x + 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y, z + 1, 0, closes) + 
                    get_light(x + 1, y - 1, z, 0, closes)) / 75.0f;
        
        float lg0 = (get_light(x + 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y, z - 1, 1, closes) + 
                    get_light(x + 1, y - 1, z, 1, closes)) / 75.0f;
        float lg1 = (get_light(x + 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y, z - 1, 1, closes) + 
                    get_light(x + 1, y + 1, z, 1, closes)) / 75.0f;
        float lg2 = (get_light(x + 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y, z + 1, 1, closes) + 
                    get_light(x + 1, y + 1, z, 1, closes)) / 75.0f;
        float lg3 = (get_light(x + 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y, z + 1, 1, closes) + 
                    get_light(x + 1, y - 1, z, 1, closes)) / 75.0f;
        
        float lb0 = (get_light(x + 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y, z - 1, 2, closes) + 
                    get_light(x + 1, y - 1, z, 2, closes)) / 75.0f;
        float lb1 = (get_light(x + 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y, z - 1, 2, closes) + 
                    get_light(x + 1, y + 1, z, 2, closes)) / 75.0f;
        float lb2 = (get_light(x + 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y, z + 1, 2, closes) + 
                    get_light(x + 1, y + 1, z, 2, closes)) / 75.0f;
        float lb3 = (get_light(x + 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y, z + 1, 2, closes) + 
                    get_light(x + 1, y - 1, z, 2, closes)) / 75.0f;
        
        float ls0 = (get_light(x + 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y, z - 1, 3, closes) + 
                    get_light(x + 1, y - 1, z, 3, closes)) / 75.0f;
        float ls1 = (get_light(x + 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y, z - 1, 3, closes) + 
                    get_light(x + 1, y + 1, z, 3, closes)) / 75.0f;
        float ls2 = (get_light(x + 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y, z + 1, 3, closes) + 
                    get_light(x + 1, y + 1, z, 3, closes)) / 75.0f;
        float ls3 = (get_light(x + 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y, z + 1, 3, closes) + 
                    get_light(x + 1, y - 1, z, 3, closes)) / 75.0f;
        
        if ((rotate == 0) || (rotate == 2)){
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u1,v1, lr3,lg3,lb3,ls3);
		} else if (rotate == 1){
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr3,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v1, lr1,lg3,lb3,ls3);
		} else if (rotate == 3){
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v2, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v1, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v2, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v1, lr3,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v1, lr1,lg3,lb3,ls3);
		}
    }
    
    // Нижняя грань (-Y)
    if (!is_blocked(x, y - 1, z, closes, group)) {
        setup_uv(textureCopyFaces[2], u1, v1, u2, v2);

        float lr = get_light(x, y - 1, z, 0, closes) / 15.0f;
        float lg = get_light(x, y - 1, z, 1, closes) / 15.0f;
        float lb = get_light(x, y - 1, z, 2, closes) / 15.0f;
        float ls = get_light(x, y - 1, z, 3, closes) / 15.0f;
        
        float lr0 = (get_light(x - 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y - 1, z, 0, closes) + 
                    get_light(x, y - 1, z - 1, 0, closes)) / 75.0f;
        float lr1 = (get_light(x + 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y - 1, z, 0, closes) + 
                    get_light(x, y - 1, z + 1, 0, closes)) / 75.0f;
        float lr2 = (get_light(x - 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y - 1, z, 0, closes) + 
                    get_light(x, y - 1, z + 1, 0, closes)) / 75.0f;
        float lr3 = (get_light(x + 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y - 1, z, 0, closes) + 
                    get_light(x, y - 1, z - 1, 0, closes)) / 75.0f;
        
        float lg0 = (get_light(x - 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y - 1, z, 1, closes) + 
                    get_light(x, y - 1, z - 1, 1, closes)) / 75.0f;
        float lg1 = (get_light(x + 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y - 1, z, 1, closes) + 
                    get_light(x, y - 1, z + 1, 1, closes)) / 75.0f;
        float lg2 = (get_light(x - 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y - 1, z, 1, closes) + 
                    get_light(x, y - 1, z + 1, 1, closes)) / 75.0f;
        float lg3 = (get_light(x + 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y - 1, z, 1, closes) + 
                    get_light(x, y - 1, z - 1, 1, closes)) / 75.0f;
        
        float lb0 = (get_light(x - 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y - 1, z, 2, closes) + 
                    get_light(x, y - 1, z - 1, 2, closes)) / 75.0f;
        float lb1 = (get_light(x + 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y - 1, z, 2, closes) + 
                    get_light(x, y - 1, z + 1, 2, closes)) / 75.0f;
        float lb2 = (get_light(x - 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y - 1, z, 2, closes) + 
                    get_light(x, y - 1, z + 1, 2, closes)) / 75.0f;
        float lb3 = (get_light(x + 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y - 1, z, 2, closes) + 
                    get_light(x, y - 1, z - 1, 2, closes)) / 75.0f;
        
        float ls0 = (get_light(x - 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y - 1, z, 3, closes) + 
                    get_light(x, y - 1, z - 1, 3, closes)) / 75.0f;
        float ls1 = (get_light(x + 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y - 1, z, 3, closes) + 
                    get_light(x, y - 1, z + 1, 3, closes)) / 75.0f;
        float ls2 = (get_light(x - 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y - 1, z, 3, closes) + 
                    get_light(x, y - 1, z + 1, 3, closes)) / 75.0f;
        float ls3 = (get_light(x + 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y - 1, z, 3, closes) + 
                    get_light(x, y - 1, z - 1, 3, closes)) / 75.0f;
        
        if ((rotate == 0) || (rotate == 2)){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		} else if (rotate == 1){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v2, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u1,v1, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v2, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u1,v1, lr1,lg1,lb1,ls1);
		} else if (rotate == 3){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		}
    }
    // Верхнаяя грань (+Y)
    if (!is_blocked(x, y + 1, z, closes, group)) {
        setup_uv(textureCopyFaces[3], u1, v1, u2, v2);

        float lr = get_light(x, y + 1, z, 0, closes) / 15.0f;
        float lg = get_light(x, y + 1, z, 1, closes) / 15.0f;
        float lb = get_light(x, y + 1, z, 2, closes) / 15.0f;
        float ls = get_light(x, y + 1, z, 3, closes) / 15.0f;
        
        float lr0 = (get_light(x - 1, y + 1, z, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y + 1, z - 1, 0, closes) + 
                    get_light(x, y + 1, z - 1, 0, closes)) / 75.0f;
        float lr1 = (get_light(x - 1, y + 1, z, 0, closes) + lr * 30.0f + 
                    get_light(x - 1, y + 1, z + 1, 0, closes) + 
                    get_light(x, y + 1, z + 1, 0, closes)) / 75.0f;
        float lr2 = (get_light(x + 1, y + 1, z, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y + 1, z + 1, 0, closes) + 
                    get_light(x, y + 1, z + 1, 0, closes)) / 75.0f;
        float lr3 = (get_light(x + 1, y + 1, z, 0, closes) + lr * 30.0f + 
                    get_light(x + 1, y + 1, z - 1, 0, closes) + 
                    get_light(x, y + 1, z - 1, 0, closes)) / 75.0f;
        
        float lg0 = (get_light(x - 1, y + 1, z, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y + 1, z - 1, 1, closes) + 
                    get_light(x, y + 1, z - 1, 1, closes)) / 75.0f;
        float lg1 = (get_light(x - 1, y + 1, z, 1, closes) + lg * 30.0f + 
                    get_light(x - 1, y + 1, z + 1, 1, closes) + 
                    get_light(x, y + 1, z + 1, 1, closes)) / 75.0f;
        float lg2 = (get_light(x + 1, y + 1, z, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y + 1, z + 1, 1, closes) + 
                    get_light(x, y + 1, z + 1, 1, closes)) / 75.0f;
        float lg3 = (get_light(x + 1, y + 1, z, 1, closes) + lg * 30.0f + 
                    get_light(x + 1, y + 1, z - 1, 1, closes) + 
                    get_light(x, y + 1, z - 1, 1, closes)) / 75.0f;
        
        float lb0 = (get_light(x - 1, y + 1, z, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y + 1, z - 1, 2, closes) + 
                    get_light(x, y + 1, z - 1, 2, closes)) / 75.0f;
        float lb1 = (get_light(x - 1, y + 1, z, 2, closes) + lb * 30.0f + 
                    get_light(x - 1, y + 1, z + 1, 2, closes) + 
                    get_light(x, y + 1, z + 1, 2, closes)) / 75.0f;
        float lb2 = (get_light(x + 1, y + 1, z, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y + 1, z + 1, 2, closes) + 
                    get_light(x, y + 1, z + 1, 2, closes)) / 75.0f;
        float lb3 = (get_light(x + 1, y + 1, z, 2, closes) + lb * 30.0f + 
                    get_light(x + 1, y + 1, z - 1, 2, closes) + 
                    get_light(x, y + 1, z - 1, 2, closes)) / 75.0f;
        
        float ls0 = (get_light(x - 1, y + 1, z, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y + 1, z - 1, 3, closes) + 
                    get_light(x, y + 1, z - 1, 3, closes)) / 75.0f;
        float ls1 = (get_light(x - 1, y + 1, z, 3, closes) + ls * 30.0f + 
                    get_light(x - 1, y + 1, z + 1, 3, closes) + 
                    get_light(x, y + 1, z + 1, 3, closes)) / 75.0f;
        float ls2 = (get_light(x + 1, y + 1, z, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y + 1, z + 1, 3, closes) + 
                    get_light(x, y + 1, z + 1, 3, closes)) / 75.0f;
        float ls3 = (get_light(x + 1, y + 1, z, 3, closes) + ls * 30.0f + 
                    get_light(x + 1, y + 1, z - 1, 3, closes) + 
                    get_light(x, y + 1, z - 1, 3, closes)) / 75.0f;
        
        if ((rotate == 0) || (rotate == 2)){
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v1, lr3,lg3,lb3,ls3);
		} else if (rotate == 1){
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v1, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr2,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr3,lg3,lb3,ls3);
		} else if (rotate == 3){
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v1, lr3,lg3,lb3,ls3);
		}
    }

    // Задняя грань (-Z)
    if (!is_blocked(x, y, z - 1, closes, group)) {
        setup_uv(textureCopyFaces[4], u1, v1, u2, v2);

        float lr = get_light(x, y, z - 1, 0, closes) / 15.0f;
        float lg = get_light(x, y, z - 1, 1, closes) / 15.0f;
        float lb = get_light(x, y, z - 1, 2, closes) / 15.0f;
        float ls = get_light(x, y, z - 1, 3, closes) / 15.0f;
        
        float lr0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y - 1, z - 1, 0, closes) + 
                        get_light(x - 1, y, z - 1, 0, closes)) / 75.0f;
        float lr1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y + 1, z - 1, 0, closes) + 
                        get_light(x - 1, y, z - 1, 0, closes)) / 75.0f;
        float lr2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y + 1, z - 1, 0, closes) + 
                        get_light(x + 1, y, z - 1, 0, closes)) / 75.0f;
        float lr3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y - 1, z - 1, 0, closes) + 
                        get_light(x + 1, y, z - 1, 0, closes)) / 75.0f;
        
        float lg0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y - 1, z - 1, 1, closes) + 
                        get_light(x - 1, y, z - 1, 1, closes)) / 75.0f;
        float lg1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y + 1, z - 1, 1, closes) + 
                        get_light(x - 1, y, z - 1, 1, closes)) / 75.0f;
        float lg2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y + 1, z - 1, 1, closes) + 
                        get_light(x + 1, y, z - 1, 1, closes)) / 75.0f;
        float lg3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y - 1, z - 1, 1, closes) + 
                        get_light(x + 1, y, z - 1, 1, closes)) / 75.0f;
        
        float lb0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y - 1, z - 1, 2, closes) + 
                        get_light(x - 1, y, z - 1, 2, closes)) / 75.0f;
        float lb1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y + 1, z - 1, 2, closes) + 
                        get_light(x - 1, y, z - 1, 2, closes)) / 75.0f;
        float lb2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y + 1, z - 1, 2, closes) + 
                        get_light(x + 1, y, z - 1, 2, closes)) / 75.0f;
        float lb3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y - 1, z - 1, 2, closes) + 
                        get_light(x + 1, y, z - 1, 2, closes)) / 75.0f;
        
        float ls0 = 0.8f * (get_light(x - 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y - 1, z - 1, 3, closes) + 
                        get_light(x - 1, y, z - 1, 3, closes)) / 75.0f;
        float ls1 = 0.8f * (get_light(x - 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y + 1, z - 1, 3, closes) + 
                        get_light(x - 1, y, z - 1, 3, closes)) / 75.0f;
        float ls2 = 0.8f * (get_light(x + 1, y + 1, z - 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y + 1, z - 1, 3, closes) + 
                        get_light(x + 1, y, z - 1, 3, closes)) / 75.0f;
        float ls3 = 0.8f * (get_light(x + 1, y - 1, z - 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y - 1, z - 1, 3, closes) + 
                        get_light(x + 1, y, z - 1, 3, closes)) / 75.0f;
        
        if ((rotate == 0) || (rotate == 2)){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u1,v1, lr3,lg3,lb3,ls3);
		} else if (rotate == 1){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u1,v1, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v2, lr3,lg3,lb3,ls3);
		} else if (rotate == 3){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, lr2,lg2,lb2,ls2);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u1,v1, lr3,lg3,lb3,ls3);
		}
    }
    // Передняя грань (+Z)
    if (!is_blocked(x, y, z + 1, closes, group)) {
        setup_uv(textureCopyFaces[5], u1, v1, u2, v2);

        float lr = get_light(x, y, z + 1, 0, closes) / 15.0f;
        float lg = get_light(x, y, z + 1, 1, closes) / 15.0f;
        float lb = get_light(x, y, z + 1, 2, closes) / 15.0f;
        float ls = get_light(x, y, z + 1, 3, closes) / 15.0f;
        
        float lr0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y - 1, z + 1, 0, closes) + 
                        get_light(x - 1, y, z + 1, 0, closes)) / 75.0f;
        float lr1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y + 1, z + 1, 0, closes) + 
                        get_light(x + 1, y, z + 1, 0, closes)) / 75.0f;
        float lr2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y + 1, z + 1, 0, closes) + 
                        get_light(x - 1, y, z + 1, 0, closes)) / 75.0f;
        float lr3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 0, closes) + lr * 30.0f + 
                        get_light(x, y - 1, z + 1, 0, closes) + 
                        get_light(x + 1, y, z + 1, 0, closes)) / 75.0f;
        
        float lg0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y - 1, z + 1, 1, closes) + 
                        get_light(x - 1, y, z + 1, 1, closes)) / 75.0f;
        float lg1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y + 1, z + 1, 1, closes) + 
                        get_light(x + 1, y, z + 1, 1, closes)) / 75.0f;
        float lg2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y + 1, z + 1, 1, closes) + 
                        get_light(x - 1, y, z + 1, 1, closes)) / 75.0f;
        float lg3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 1, closes) + lg * 30.0f + 
                        get_light(x, y - 1, z + 1, 1, closes) + 
                        get_light(x + 1, y, z + 1, 1, closes)) / 75.0f;
        
        float lb0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y - 1, z + 1, 2, closes) + 
                        get_light(x - 1, y, z + 1, 2, closes)) / 75.0f;
        float lb1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y + 1, z + 1, 2, closes) + 
                        get_light(x + 1, y, z + 1, 2, closes)) / 75.0f;
        float lb2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y + 1, z + 1, 2, closes) + 
                        get_light(x - 1, y, z + 1, 2, closes)) / 75.0f;
        float lb3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 2, closes) + lb * 30.0f + 
                        get_light(x, y - 1, z + 1, 2, closes) + 
                        get_light(x + 1, y, z + 1, 2, closes)) / 75.0f;
        
        float ls0 = 0.9f * (get_light(x - 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y - 1, z + 1, 3, closes) + 
                        get_light(x - 1, y, z + 1, 3, closes)) / 75.0f;
        float ls1 = 0.9f * (get_light(x + 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y + 1, z + 1, 3, closes) + 
                        get_light(x + 1, y, z + 1, 3, closes)) / 75.0f;
        float ls2 = 0.9f * (get_light(x - 1, y + 1, z + 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y + 1, z + 1, 3, closes) + 
                        get_light(x - 1, y, z + 1, 3, closes)) / 75.0f;
        float ls3 = 0.9f * (get_light(x + 1, y - 1, z + 1, 3, closes) + ls * 30.0f + 
                        get_light(x, y - 1, z + 1, 3, closes) + 
                        get_light(x + 1, y, z + 1, 3, closes)) / 75.0f;
        
        if ((rotate == 0) || (rotate == 2)){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v1, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		} else if (rotate == 1){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v1, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u1,v2, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		} else if (rotate == 3){
			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
			add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u1,v2, lr2,lg2,lb2,ls2);

			add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, lr0,lg0,lb0,ls0);
			add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v1, lr3,lg3,lb3,ls3);
			add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, lr1,lg1,lb1,ls1);
		}
    }
}

// void _renderBlockShadeless(std::vector<float>& buffer, int x, int y, int z, const Chunk** closes, uint voxel_id, size_t& index) {
//     float u1, v1, u2, v2;

//     Block* block = Block::blocks[voxel_id];
// 	ubyte group = block->drawGroup;

//     if (!is_blocked(x - 1, y, z, closes, group)){
//         setup_uv(block->textureFaces[0], u1, v1, u2, v2);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u1,v2, 1,1,1,1);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,1);
// 	}
//     if (!is_blocked(x + 1, y, z, closes, group)){
// 		setup_uv(block->textureFaces[1], u1, v1, u2, v2);

// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u2,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,1);

// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u1,v1, 1,1,1,1);
// 	}

//     if (!is_blocked(x, y - 1, z, closes, group)){
// 		setup_uv(block->textureFaces[2], u1, v1, u2, v2);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v2, 1,1,1,1);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v2, 1,1,1,1);
// 	}
//     if (!is_blocked(x, y + 1, z, closes, group)){
// 		setup_uv(block->textureFaces[3], u1, v1, u2, v2);

// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,1);

// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v1, 1,1,1,1);
// 	}

//     if (!is_blocked(x, y, z - 1, closes, group)){
// 		setup_uv(block->textureFaces[4], u1, v1, u2, v2);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z-0.5f, u2,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, 1,1,1,1);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z-0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z-0.5f, u1,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z-0.5f, u1,v1, 1,1,1,1);
// 	}
// 	if (!is_blocked(x, y, z + 1, closes, group)){
// 		setup_uv(block->textureFaces[5], u1, v1, u2, v2);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,1);
// 		add_vertex(buffer, index, x-0.5f, y+0.5f, z+0.5f, u1,v2, 1,1,1,1);

// 		add_vertex(buffer, index, x-0.5f, y-0.5f, z+0.5f, u1,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y-0.5f, z+0.5f, u2,v1, 1,1,1,1);
// 		add_vertex(buffer, index, x+0.5f, y+0.5f, z+0.5f, u2,v2, 1,1,1,1);
// 	}
// }

void _renderXBlock(std::vector<float>& buffer, int x, int y, int z, const Chunk** closes, uint voxel_id, size_t& index){
	Block* block = Block::blocks[voxel_id].get();

	if (block->model != Block_models::X) return;

    int rand = ((x * z + y) xor (z * y - x)) * (z + y);

	float xs = (float)(char)rand / 512;
	float zs = (float)(char)(rand >> 8) / 512;

    float u1, v1, u2, v2;

	float lr = get_light(x,y,z, 0, closes) / 15.0f;
	float lg = get_light(x,y,z, 1, closes) / 15.0f;
	float lb = get_light(x,y,z, 2, closes) / 15.0f;
	float ls = get_light(x,y,z, 3, closes) / 15.0f;

	float lr0 = (get_light(x,y-1,z,0, closes) + lr*30) / 45.0f;
	float lr1 = (get_light(x,y+1,z,0, closes) + lr*30) / 45.0f;
	float lr2 = (get_light(x,y+1,z,0, closes) + lr*30) / 45.0f;
	float lr3 = (get_light(x,y-1,z,0, closes) + lr*30) / 45.0f;
	float lr4 = (get_light(x,y-1,z,0, closes) + lr*30) / 45.0f;
	float lr5 = (get_light(x,y+1,z,0, closes) + lr*30) / 45.0f;
	float lr6 = (get_light(x,y+1,z,0, closes) + lr*30) / 45.0f;
	float lr7 = (get_light(x,y-1,z,0, closes) + lr*30) / 45.0f;

	float lg0 = (get_light(x,y-1,z,1, closes) + lg*30) / 45.0f;
	float lg1 = (get_light(x,y+1,z,1, closes) + lg*30) / 45.0f;
	float lg2 = (get_light(x,y+1,z,1, closes) + lg*30) / 45.0f;
	float lg3 = (get_light(x,y-1,z,1, closes) + lg*30) / 45.0f;
	float lg4 = (get_light(x,y-1,z,1, closes) + lg*30) / 45.0f;
	float lg5 = (get_light(x,y+1,z,1, closes) + lg*30) / 45.0f;
	float lg6 = (get_light(x,y+1,z,1, closes) + lg*30) / 45.0f;
	float lg7 = (get_light(x,y-1,z,1, closes) + lg*30) / 45.0f;

	float lb0 = (get_light(x,y-1,z,2, closes) + lb*30) / 45.0f;
	float lb1 = (get_light(x,y+1,z,2, closes) + lb*30) / 45.0f;
	float lb2 = (get_light(x,y+1,z,2, closes) + lb*30) / 45.0f;
	float lb3 = (get_light(x,y-1,z,2, closes) + lb*30) / 45.0f;
	float lb4 = (get_light(x,y-1,z,2, closes) + lb*30) / 45.0f;
	float lb5 = (get_light(x,y+1,z,2, closes) + lb*30) / 45.0f;
	float lb6 = (get_light(x,y+1,z,2, closes) + lb*30) / 45.0f;
	float lb7 = (get_light(x,y-1,z,2, closes) + lb*30) / 45.0f;

	float ls0 = (get_light(x,y-1,z,3, closes) + ls*30) / 45.0f;
	float ls1 = (get_light(x,y+1,z,3, closes) + ls*30) / 45.0f;
	float ls2 = (get_light(x,y+1,z,3, closes) + ls*30) / 45.0f;
	float ls3 = (get_light(x,y-1,z,3, closes) + ls*30) / 45.0f;
	float ls4 = (get_light(x,y-1,z,3, closes) + ls*30) / 45.0f;
	float ls5 = (get_light(x,y+1,z,3, closes) + ls*30) / 45.0f;
	float ls6 = (get_light(x,y+1,z,3, closes) + ls*30) / 45.0f;
	float ls7 = (get_light(x,y-1,z,3, closes) + ls*30) / 45.0f;

	{
        setup_uv(block->textureFaces[1], u1, v1, u2, v2);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z-0.5f+zs, u2,v1, lr0,lg0,lb0,ls0);
        add_vertex(buffer, index, x-0.5f+xs, y+0.5f, z-0.5f+zs, u2,v2, lr1,lg1,lb1,ls1);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z+0.5f+zs, u1,v2, lr2,lg2,lb2,ls2);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z-0.5f+zs, u2,v1, lr0,lg0,lb0,ls0);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z+0.5f+zs, u1,v2, lr2,lg2,lb2,ls2);
        add_vertex(buffer, index, x+0.5f+xs, y-0.5f, z+0.5f+zs, u1,v1, lr3,lg3,lb3,ls3);
    }

	{
        setup_uv(block->textureFaces[0], u1, v1, u2, v2);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z-0.5f+zs, u1,v1, lr0,lg0,lb0,ls0);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z+0.5f+zs, u2,v2, lr1,lg1,lb1,ls1);
        add_vertex(buffer, index, x-0.5f+xs, y+0.5f, z-0.5f+zs, u1,v2, lr2,lg2,lb2,ls2);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z-0.5f+zs, u1,v1, lr0,lg0,lb0,ls0);
        add_vertex(buffer, index, x+0.5f+xs, y-0.5f, z+0.5f+zs, u2,v1, lr3,lg3,lb3,ls3);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z+0.5f+zs, u2,v2, lr1,lg1,lb1,ls1);
    }

	{
        setup_uv(block->textureFaces[5], u1, v1, u2, v2);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z+0.5f+zs, u1,v1, lr4,lg4,lb4,ls4);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z-0.5f+zs, u2,v2, lr5,lg5,lb5,ls5);
        add_vertex(buffer, index, x-0.5f+xs, y+0.5f, z+0.5f+zs, u1,v2, lr6,lg6,lb6,ls6);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z+0.5f+zs, u1,v1, lr4,lg4,lb4,ls4);
        add_vertex(buffer, index, x+0.5f+xs, y-0.5f, z-0.5f+zs, u2,v1, lr7,lg7,lb7,ls7);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z-0.5f+zs, u2,v2, lr5,lg5,lb5,ls5);
    }

	{
        setup_uv(block->textureFaces[4], u1, v1, u2, v2);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z+0.5f+zs, u2,v1, lr4,lg4,lb4,ls4);
        add_vertex(buffer, index, x-0.5f+xs, y+0.5f, z+0.5f+zs, u2,v2, lr5,lg5,lb5,ls5);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z-0.5f+zs, u1,v2, lr6,lg6,lb6,ls6);

        add_vertex(buffer, index, x-0.5f+xs, y-0.5f, z+0.5f+zs, u2,v1, lr4,lg4,lb4,ls4);
        add_vertex(buffer, index, x+0.5f+xs, y+0.5f, z-0.5f+zs, u1,v2, lr6,lg6,lb6,ls6);
        add_vertex(buffer, index, x+0.5f+xs, y-0.5f, z-0.5f+zs, u1,v1, lr7,lg7,lb7,ls7);
    }
}


VoxelRenderer::VoxelRenderer() {
}

VoxelRenderer::~VoxelRenderer(){
}

const float* VoxelRenderer::render(Chunk* chunk, const Chunk** closes, size_t& size){
	buffer.clear();
	size_t index = 0;
	for (int y = 0; y < CHUNK_HEIGHT; ++y){
		for (int z = 0; z < CHUNK_DEPTH; ++z){
			for (int x = 0; x < CHUNK_WIDTH; ++x){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				if (vox.id == Blocks_id::AIR || vox.id == Blocks_id::GLASS || vox.id == Blocks_id::WATER) continue;
				Block* block = Block::blocks[vox.id].get();
				if (block->model != Block_models::CUBE) continue;
                // if (block->emission[0] || block->emission[1] || block->emission[2]) continue;
				
				_renderBlock(buffer, x, y, z, closes, vox, index);
			}
		}
	}

	// for (int y = 0; y < CHUNK_HEIGHT; ++y){
	// 	for (int z = 0; z < CHUNK_DEPTH; ++z){
	// 		for (int x = 0; x < CHUNK_WIDTH; ++x){
	// 			voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
	// 			if (vox.id == Blocks_id::AIR) continue;
	// 			Block* block = Block::blocks[vox.id];
	// 			if (block->emission[0] || block->emission[1] || block->emission[2]) {
	// 				_renderBlockShadeless(buffer, x, y, z, closes, vox.id, index);
	// 			}
	// 		}
	// 	}
	// }

	for (int y = 0; y < CHUNK_HEIGHT; ++y){
		for (int z = 0; z < CHUNK_DEPTH; ++z){
			for (int x = 0; x < CHUNK_WIDTH; ++x){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				if (vox.id != Blocks_id::WATER) continue;
				_renderBlock(buffer, x, y, z, closes, vox, index);
			}
		}
	}

	for (int y = 0; y < CHUNK_HEIGHT; ++y){
		for (int z = 0; z < CHUNK_DEPTH; ++z){
			for (int x = 0; x < CHUNK_WIDTH; ++x){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				if (Block::blocks[vox.id].get()->model != Block_models::X) continue;
				_renderXBlock(buffer, x, y, z, closes, vox.id, index);
			}
		}
	}

	for (int y = 0; y < CHUNK_HEIGHT; ++y){
		for (int z = 0; z < CHUNK_DEPTH; ++z){
			for (int x = 0; x < CHUNK_WIDTH; ++x){
				voxel vox = chunk->voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x];
				if (vox.id != Blocks_id::GLASS) continue;
				_renderBlock(buffer, x, y, z, closes, vox, index);
			}
		}
	}
	size = buffer.size();
	return &buffer[0];
}
