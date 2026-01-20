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
    // Воздух
    {
        auto block = std::make_unique<Block>(Blocks_id::AIR, 0);
        block->drawGroup = 1;
        block->lightPassing = true;
        block->skyLightPassing = true;
        block->obstacle = false;
        block->selectable = false;
        block->model = Block_models::AIR;
        Block::blocks[block->id] = std::move(block);
    }

    // Мох
    {
        auto block = std::make_unique<Block>(Blocks_id::MOSS, 1);
        Block::blocks[block->id] = std::move(block);
    }

    // Земля
    {
        auto block = std::make_unique<Block>(Blocks_id::DIRT, 2);
        Block::blocks[block->id] = std::move(block);
    }

    // Светокамень
    {
        auto block = std::make_unique<Block>(Blocks_id::GLOWSTONE, 3);
        block->emission[0] = 14;
        block->emission[1] = 12;
        block->emission[2] = 3;
        Block::blocks[block->id] = std::move(block);
    }

    // Стекло
    {
        auto block = std::make_unique<Block>(Blocks_id::GLASS, 4);
        block->drawGroup = 2;
        block->lightPassing = true;
        Block::blocks[block->id] = std::move(block);
    }

    // Доски
    {
        auto block = std::make_unique<Block>(Blocks_id::PLANKS, 5);
        Block::blocks[block->id] = std::move(block);
    }

    // Бревно
    {
        auto block = std::make_unique<Block>(Blocks_id::LOG, 6);
        block->textureFaces[2] = 7;
        block->textureFaces[3] = 7;
        block->rotatable = true;
        Block::blocks[block->id] = std::move(block);
    }

    // Листва
    {
        auto block = std::make_unique<Block>(Blocks_id::LEAVES, 8);
        Block::blocks[block->id] = std::move(block);
    }

    // Вода
    {
        auto block = std::make_unique<Block>(Blocks_id::WATER, 9);
        block->drawGroup = 4;
        block->lightPassing = true;
        block->skyLightPassing = false;
        block->obstacle = false;
        block->selectable = false;
        Block::blocks[block->id] = std::move(block);
    }

    // Камень
    {
        auto block = std::make_unique<Block>(Blocks_id::STONE, 10);
        Block::blocks[block->id] = std::move(block);
    }

    // Песок
    {
        auto block = std::make_unique<Block>(Blocks_id::SAND, 11);
        Block::blocks[block->id] = std::move(block);
    }

    // Коренная порода
    {
        auto block = std::make_unique<Block>(Blocks_id::BEDROCK, 12);
        block->breakable = false;
        Block::blocks[block->id] = std::move(block);
    }

    // Мак
    {
        auto block = std::make_unique<Block>(Blocks_id::POPPY, 13);
        block->drawGroup = 5;
        block->lightPassing = true;
        block->obstacle = false;
        block->model = Block_models::X;
        Block::blocks[block->id] = std::move(block);
    }

    // Одуванчик
    {
        auto block = std::make_unique<Block>(Blocks_id::DANDELION, 14);
        block->drawGroup = 5;
        block->lightPassing = true;
        block->obstacle = false;
        block->model = Block_models::X;
        Block::blocks[block->id] = std::move(block);
    }

    // Трава
    {
        auto block = std::make_unique<Block>(Blocks_id::GRASS, 15);
        block->drawGroup = 5;
        block->lightPassing = true;
        block->obstacle = false;
        block->model = Block_models::X;
        Block::blocks[block->id] = std::move(block);
    }

    // Кирпичи
    {
        auto block = std::make_unique<Block>(Blocks_id::BRICKS, 16);
        Block::blocks[block->id] = std::move(block);
    }

    // Ромашка
    {
        auto block = std::make_unique<Block>(Blocks_id::DAISY, 17);
        block->drawGroup = 5;
        block->lightPassing = true;
        block->obstacle = false;
        block->model = Block_models::X;
        Block::blocks[block->id] = std::move(block);
    }

    // Красный неоновый блок
    {
        auto block = std::make_unique<Block>(Blocks_id::RED_NEON, 18);
        block->emission[0] = 15;
        Block::blocks[block->id] = std::move(block);
    }

    // Зелёный неоновый блок
    {
        auto block = std::make_unique<Block>(Blocks_id::GREEN_NEON, 19);
        block->emission[1] = 15;
        Block::blocks[block->id] = std::move(block);
    }

    // Синий неоновый блок
    {
        auto block = std::make_unique<Block>(Blocks_id::BLUE_NEON, 20);
        block->emission[2] = 15;
        Block::blocks[block->id] = std::move(block);
    }
}
