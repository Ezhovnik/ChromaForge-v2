#include "definitions.h"

#include <glm/glm.hpp>

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
    Block* block = new Block(DEFAULT_BLOCK_NAMESPACE"air");
    block->drawGroup = 1;
    block->lightPassing = true;
    block->skyLightPassing = true;
    block->obstacle = false;
    block->selectable = false;
    block->replaceable = true;
    block->model = BlockModel::None;
    builder->add(block);

    // Мох
    block = new Block(DEFAULT_BLOCK_NAMESPACE"moss", "moss_block");
    builder->add(block);

    // Земля
    block = new Block(DEFAULT_BLOCK_NAMESPACE"dirt", "dirt");
    builder->add(block);

    // Светокамень
    block = new Block(DEFAULT_BLOCK_NAMESPACE"glowstone", "glowstone");
    block->emission[0] = 14;
    block->emission[1] = 12;
    block->emission[2] = 3;
    builder->add(block);

    // Стекло
    block = new Block(DEFAULT_BLOCK_NAMESPACE"glass", "glass");
    block->drawGroup = 2;
    block->lightPassing = true;
    builder->add(block);

    // Доски
    block = new Block(DEFAULT_BLOCK_NAMESPACE"planks", "oak_planks");
    builder->add(block);

    // Бревно
    block = new Block(DEFAULT_BLOCK_NAMESPACE"log", "oak_log");
    block->textureFaces[2] = "oak_log_top";
    block->textureFaces[3] = "oak_log_top";
    block->rotatable = true;
    builder->add(block);

    // Листва
    block = new Block(DEFAULT_BLOCK_NAMESPACE"leaves", "leaves");
    builder->add(block);

    // Вода
    block = new Block(DEFAULT_BLOCK_NAMESPACE"water", "water");
    block->drawGroup = 4;
    block->lightPassing = true;
    block->skyLightPassing = false;
    block->obstacle = false;
    block->selectable = false;
    builder->add(block);

    // Камень
    block = new Block(DEFAULT_BLOCK_NAMESPACE"stone", "stone");
    builder->add(block);

    // Песок
    block = new Block(DEFAULT_BLOCK_NAMESPACE"sand", "sand");
    builder->add(block);

    // Коренная порода
    block = new Block(DEFAULT_BLOCK_NAMESPACE"bedrock", "bedrock");
    block->breakable = false;
    builder->add(block);

    // Мак
    block = new Block(DEFAULT_BLOCK_NAMESPACE"poppy", "poppy");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->replaceable = true;
    block->hitbox.scale(glm::vec3(0.7f));
    block->model = BlockModel::X;
    builder->add(block);

    // Одуванчик
    block = new Block(DEFAULT_BLOCK_NAMESPACE"dandelion", "dandelion");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->replaceable = true;
    block->hitbox.scale(glm::vec3(0.7f));
    block->model = BlockModel::X;
    builder->add(block);

    // Трава
    block = new Block(DEFAULT_BLOCK_NAMESPACE"grass", "grass");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->replaceable = true;
    block->hitbox.scale(glm::vec3(0.7f), glm::vec3(0.5f, 0.0f, 0.5f));
    block->model = BlockModel::X;
    builder->add(block);

    // Кирпичи
    block = new Block(DEFAULT_BLOCK_NAMESPACE"bricks", "bricks");
    builder->add(block);

    // Ромашка
    block = new Block(DEFAULT_BLOCK_NAMESPACE"daisy", "daisy");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->replaceable = true;
    block->hitbox.scale(glm::vec3(0.7f));
    block->model = BlockModel::X;
    builder->add(block);

    // Бархатцы
    block = new Block(DEFAULT_BLOCK_NAMESPACE"marigold", "marigold");
    block->drawGroup = 5;
    block->lightPassing = true;
    block->obstacle = false;
    block->replaceable = true;
    block->hitbox.scale(glm::vec3(0.7f));
    block->model = BlockModel::X;
    builder->add(block);

    // Красный неоновый блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE"red_neon", "red_neon");
    block->emission[0] = 15;
    builder->add(block);

    // Зелёный неоновый блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE"green_neon", "green_neon");
    block->emission[1] = 15;
    builder->add(block);

    // Синий неоновый блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE"blue_neon", "blue_neon");
    block->emission[2] = 15;
    builder->add(block);

    // Булыжник
    block = new Block(DEFAULT_BLOCK_NAMESPACE"cobblestone", "cobblestone");
    builder->add(block);

    // Медный блок
    block = new Block(DEFAULT_BLOCK_NAMESPACE"copper_block", "copper_block");
    builder->add(block);

    // Базальт
    block = new Block(DEFAULT_BLOCK_NAMESPACE"basalt", "basalt");
    block->textureFaces[2] = "basalt_top";
    block->textureFaces[3] = "basalt_top";
    block->rotatable = true;
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
