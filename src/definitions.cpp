#include "definitions.h"

#include <iostream>

#include "assets/AssetsLoader.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/Font.h"
#include "window/Window.h"
#include "voxels/Block.h"
#include "logger/Logger.h"

void setup_definitions() {
    for (size_t i = 0; i < 256; i++) {
		Block::blocks[i] = nullptr;
	}

    // Воздух
    Block* block = new Block(Blocks_id::AIR, 0);
    block->drawGroup = 1;
    block->lightPassing = true;
    block->skyLightPassing = true;
    block->obstacle = false;
    block->selectable = false;
    block->model = BlockModel::None;
    Block::blocks[block->id] = block;

    // Мох
    block = new Block(Blocks_id::MOSS, 1);
    Block::blocks[block->id] = block;

    // Земля
    block = new Block(Blocks_id::DIRT, 2);
    Block::blocks[block->id] = block;

    // Светокамень
    block = new Block(Blocks_id::GLOWSTONE, 3);
    block->emission[0] = 14;
    block->emission[1] = 12;
    block->emission[2] = 3;
    Block::blocks[block->id] = block;

    // Стекло
    block = new Block(Blocks_id::GLASS, 4);
    block->drawGroup = 2;
    block->lightPassing = true;
    Block::blocks[block->id] = block;

    // Доски
    block = new Block(Blocks_id::PLANKS, 5);
    Block::blocks[block->id] = block;

    // Бревно
    block = new Block(Blocks_id::LOG, 6);
    block->textureFaces[2] = 7;
    block->textureFaces[3] = 7;
    block->rotatable = true;
    Block::blocks[block->id] = block;

    // Листва
    block = new Block(Blocks_id::LEAVES, 8);
    Block::blocks[block->id] = block;

    // Вода
    block = new Block(Blocks_id::WATER, 9);
    block->drawGroup = 4;
    block->lightPassing = true;
    block->skyLightPassing = false;
    block->obstacle = false;
    block->selectable = false;
    Block::blocks[block->id] = block;

    // Камень
    block = new Block(Blocks_id::STONE, 10);
    Block::blocks[block->id] = block;

    // Песок
    block = new Block(Blocks_id::SAND, 11);
    Block::blocks[block->id] = block;

    // Коренная порода
    block = new Block(Blocks_id::BEDROCK, 12);
    block->breakable = false;
    Block::blocks[block->id] = block;

    // Мак
    block = new Block(Blocks_id::POPPY, 13);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Одуванчик
    block = new Block(Blocks_id::DANDELION, 14);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Трава
    block = new Block(Blocks_id::GRASS, 15);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->hitboxScale = 0.5f;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Кирпичи
    block = new Block(Blocks_id::BRICKS, 16);
    Block::blocks[block->id] = block;


    // Ромашка
    block = new Block(Blocks_id::DAISY, 17);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Бархатцы
    block = new Block(Blocks_id::MARIGOLD, 21);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Красный неоновый блок
    block = new Block(Blocks_id::RED_NEON, 18);
    block->emission[0] = 15;
    Block::blocks[block->id] = block;

    // Зелёный неоновый блок
    block = new Block(Blocks_id::GREEN_NEON, 19);
    block->emission[1] = 15;
    Block::blocks[block->id] = block;

    // Синий неоновый блок
    block = new Block(Blocks_id::BLUE_NEON, 20);
    block->emission[2] = 15;
    Block::blocks[block->id] = block;

    // Булыжник
    block = new Block(Blocks_id::COBBLESTONE, 22);
    Block::blocks[block->id] = block;
}
