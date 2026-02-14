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
#include "content/Content.h"

void setup_definitions(ContentBuilder* builder) {
    // Воздух
    Block* block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("air"));
    block->drawGroup = 1;
    block->lightPassing = true;
    block->skyLightPassing = true;
    block->obstacle = false;
    block->selectable = false;
    block->model = BlockModel::None;
    builder->add(block);

    // Мох
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("moss"), "moss_block");
    builder->add(block);

    // Земля
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("dirt"), "dirt");
    builder->add(block);

    // Светокамень
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("glowstone"), "glowstone");
    block->emission[0] = 14;
    block->emission[1] = 12;
    block->emission[2] = 3;
    builder->add(block);

    // Стекло
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("glass"), "glass");
    block->drawGroup = 2;
    block->lightPassing = true;
    builder->add(block);

    // Доски
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("planks"), "oak_planks");
    builder->add(block);

    // Бревно
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("log"), "oak_log");
    block->textureFaces[2] = "oak_log_top";
    block->textureFaces[3] = "oak_log_top";
    block->rotatable = true;
    builder->add(block);

    // Листва
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("leaves"), "leaves");
    builder->add(block);

    // Вода
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("water"), "water");
    block->drawGroup = 4;
    block->lightPassing = true;
    block->skyLightPassing = false;
    block->obstacle = false;
    block->selectable = false;
    builder->add(block);

    // Камень
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("stone"), "stone");
    builder->add(block);

    // Песок
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("sand"), "sand");
    builder->add(block);

    // Коренная порода
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("bedrock"), "bedrock");
    block->breakable = false;
    builder->add(block);

    // Мак
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("poppy"), "poppy");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    builder->add(block);

    // Одуванчик
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("dandelion"), "dandelion");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    builder->add(block);

    // Трава
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("grass"), "grass");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->hitboxScale = 0.5f;
    block->model = BlockModel::X;
    builder->add(block);

    // Кирпичи
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("bricks"), "bricks");
    builder->add(block);

    // Ромашка
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("daisy"), "daisy");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    builder->add(block);

    // Бархатцы
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("marigold"), "marigold");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->model = BlockModel::X;
    builder->add(block);

    // Красный неоновый блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("red_neon"), "red_neon");
    block->emission[0] = 15;
    builder->add(block);

    // Зелёный неоновый блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("green_neon"), "green_neon");
    block->emission[1] = 15;
    builder->add(block);

    // Синий неоновый блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("blue_neon"), "blue_neon");
    block->emission[2] = 15;
    builder->add(block);

    // Булыжник
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("cobblestone"), "cobblestone");
    builder->add(block);

    // Медный блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE + std::string("copper_block"), "copper_block");
    builder->add(block);
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

    Events::bind(BIND_PLAYER_ATTACK, inputType::mouse, mousecode::BUTTON_1);
    Events::bind(BIND_PLAYER_BUILD, inputType::mouse, mousecode::BUTTON_2);
    Events::bind(BIND_PLAYER_PICK, inputType::mouse, mousecode::BUTTON_3);
}
