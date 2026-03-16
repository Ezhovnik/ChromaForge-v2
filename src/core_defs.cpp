#include <glm/glm.hpp>

#include "core_content_defs.h"
#include "input_bindings.h"
#include "assets/AssetsLoader.h"
#include "graphics/core/ShaderProgram.h"
#include "graphics/core/Texture.h"
#include "graphics/core/Font.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/input.h"
#include "voxels/Block.h"
#include "debug/Logger.h"
#include "content/Content.h"
#include "items/Item.h"

void CoreContent::setup(ContentBuilder* builder) {
    // Воздух
    Block& block = builder->createBlock(BUILTIN_AIR);
    block.drawGroup = 1;
    block.lightPassing = true;
    block.skyLightPassing = true;
    block.obstacle = false;
    block.selectable = false;
    block.replaceable = true;
    block.model = BlockModel::None;
    block.pickingItem = BUILTIN_EMPTY;

    // Пустота
    Item& item = builder->createItem(BUILTIN_EMPTY);
    item.iconType = ItemIconType::None;
}

void CoreContent::setup_bindings() {
	Events::bind(BIND_MOVE_FORWARD, inputType::keyboard, keycode::W);
	Events::bind(BIND_MOVE_BACK, inputType::keyboard, keycode::S);
	Events::bind(BIND_MOVE_RIGHT, inputType::keyboard, keycode::D);
	Events::bind(BIND_MOVE_LEFT, inputType::keyboard, keycode::A);
	Events::bind(BIND_MOVE_JUMP, inputType::keyboard, keycode::SPACE);
	Events::bind(BIND_MOVE_SPRINT, inputType::keyboard, keycode::LEFT_CONTROL);
	Events::bind(BIND_MOVE_CROUCH, inputType::keyboard, keycode::LEFT_SHIFT);
	Events::bind(BIND_MOVE_CHEAT, inputType::keyboard, keycode::R);
	Events::bind(BIND_CAM_ZOOM, inputType::keyboard, keycode::C);
    Events::bind(BIND_CAM_MODE, inputType::keyboard, keycode::F4);
	Events::bind(BIND_PLAYER_NOCLIP, inputType::keyboard, keycode::N);
	Events::bind(BIND_PLAYER_FLIGHT, inputType::keyboard, keycode::F);
    Events::bind(BIND_HUD_INVENTORY, inputType::keyboard, keycode::E);

    Events::bind(BIND_PLAYER_ATTACK, inputType::mouse, mousecode::BUTTON_1);
    Events::bind(BIND_PLAYER_BUILD, inputType::mouse, mousecode::BUTTON_2);
    Events::bind(BIND_PLAYER_PICK, inputType::mouse, mousecode::BUTTON_3);
}
