#include <glm/glm.hpp>

#include <core_content_defs.h>
#include <input_bindings.h>
#include <assets/AssetsLoader.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/Font.h>
#include <window/Window.h>
#include <window/Events.h>
#include <window/input.h>
#include <voxels/Block.h>
#include <debug/Logger.h>
#include <content/Content.h>
#include <items/Item.h>
#include <content/ContentBuilder.h>
#include <files/files.h>
#include <files/engine_paths.h>

void CoreContent::setup(EnginePaths* paths, ContentBuilder* builder) {
    // Воздух
    Block& block = builder->blocks.create(BUILTIN_AIR);
    block.drawGroup = 1;
    block.lightPassing = true;
    block.skyLightPassing = true;
    block.obstacle = false;
    block.selectable = false;
    block.replaceable = true;
    block.model = BlockModel::None;
    block.pickingItem = BUILTIN_EMPTY;

    // Пустота
    Item& item = builder->items.create(BUILTIN_EMPTY);
    item.iconType = ItemIconType::None;
}
