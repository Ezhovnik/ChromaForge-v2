#include "definitions.h"

#include <iostream>

#include "assets/AssetsLoader.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/Font.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/input.h"
#include "voxels/Block.h"
#include "logger/Logger.h"
#include "core_defs.h"

void setup_definitions() {
    for (size_t i = 0; i < 256; i++) {
		Block::blocks[i] = nullptr;
    }

    // Воздух
    Block* block = new Block(BlockID::AIR, 0);
    block->drawGroup = 1;
    block->lightPassing = true;
    block->skyLightPassing = true;
    block->obstacle = false;
    block->selectable = false;
    block->model = BlockModel::None;
    Block::blocks[block->id] = block;

    // Мох
    block = new Block(BlockID::MOSS, 1);
    Block::blocks[block->id] = block;

    // Земля
    block = new Block(BlockID::DIRT, 2);
    Block::blocks[block->id] = block;

    // Светокамень
    block = new Block(BlockID::GLOWSTONE, 3);
    block->emission[0] = 14;
    block->emission[1] = 12;
    block->emission[2] = 3;
    Block::blocks[block->id] = block;

    // Стекло
    block = new Block(BlockID::GLASS, 4);
    block->drawGroup = 2;
    block->lightPassing = true;
    Block::blocks[block->id] = block;

    // Доски
    block = new Block(BlockID::PLANKS, 5);
    Block::blocks[block->id] = block;

    // Бревно
    block = new Block(BlockID::LOG, 6);
    block->textureFaces[2] = 7;
    block->textureFaces[3] = 7;
    block->rotatable = true;
    Block::blocks[block->id] = block;

    // Листва
    block = new Block(BlockID::LEAVES, 8);
    Block::blocks[block->id] = block;

    // Вода
    block = new Block(BlockID::WATER, 9);
    block->drawGroup = 4;
    block->lightPassing = true;
    block->skyLightPassing = false;
    block->obstacle = false;
    block->selectable = false;
    Block::blocks[block->id] = block;

    // Камень
    block = new Block(BlockID::STONE, 10);
    Block::blocks[block->id] = block;

    // Песок
    block = new Block(BlockID::SAND, 11);
    Block::blocks[block->id] = block;

    // Коренная порода
    block = new Block(BlockID::BEDROCK, 12);
    block->breakable = false;
    Block::blocks[block->id] = block;

    // Мак
    block = new Block(BlockID::POPPY, 13);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Одуванчик
    block = new Block(BlockID::DANDELION, 14);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Трава
    block = new Block(BlockID::GRASS, 15);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->hitboxScale = 0.5f;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Кирпичи
    block = new Block(BlockID::BRICKS, 16);
    Block::blocks[block->id] = block;

    // Ромашка
    block = new Block(BlockID::DAISY, 17);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Бархатцы
    block = new Block(BlockID::MARIGOLD, 21);
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    Block::blocks[block->id] = block;

    // Красный неоновый блок
    block = new Block(BlockID::RED_NEON, 18);
    block->emission[0] = 15;
    Block::blocks[block->id] = block;

    // Зелёный неоновый блок
    block = new Block(BlockID::GREEN_NEON, 19);
    block->emission[1] = 15;
    Block::blocks[block->id] = block;

    // Синий неоновый блок
    block = new Block(BlockID::BLUE_NEON, 20);
    block->emission[2] = 15;
    Block::blocks[block->id] = block;

    // Булыжник
    block = new Block(BlockID::COBBLESTONE, 22);
    Block::blocks[block->id] = block;
}

void setup_bindings() {
	Events::bind(BIND_MOVE_FORWARD, inputType::keyboard, keycode::W);
	Events::bind(BIND_MOVE_BACK, inputType::keyboard, keycode::S);
	Events::bind(BIND_MOVE_RIGHT, inputType::keyboard, keycode::D);
	Events::bind(BIND_MOVE_LEFT, inputType::keyboard, keycode::A);
	Events::bind(BIND_MOVE_JUMP, inputType::keyboard, keycode::SPACE);
	Events::bind(BIND_MOVE_SPRINT, inputType::keyboard, keycode::LEFT_CONTROL);
	Events::bind(BIND_MOVE_CROUCH, inputType::keyboard, keycode::LEFT_SHIFT);
	Events::bind(BIND_MOVE_CHEAT, inputType::keyboard, keycode::R);
	Events::bind(BIND_CAM_ZOOM, inputType::keyboard, keycode::C);
	Events::bind(BIND_PLAYER_NOCLIP, inputType::keyboard, keycode::N);
	Events::bind(BIND_PLAYER_FLIGHT, inputType::keyboard, keycode::F);
    Events::bind(BIND_HUD_INVENTORY, inputType::keyboard, keycode::E);

    Events::bind(BIND_BLOCK_BREAK, inputType::mouse, mousecode::BUTTON_1);
    Events::bind(BIND_BLOCK_SET, inputType::mouse, mousecode::BUTTON_2);
    Events::bind(BIND_BLOCK_SELECT, inputType::mouse, mousecode::BUTTON_3);
}
