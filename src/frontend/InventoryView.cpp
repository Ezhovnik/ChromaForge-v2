#include "InventoryView.h"

#include <glm/glm.hpp>

#include "BlocksPreview.h"
#include "../window/Events.h"
#include "../assets/Assets.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Batch2D.h"
#include "../graphics/GfxContext.h"
#include "../content/Content.h"
#include "../math/voxmaths.h"
#include "../objects/Player.h"
#include "../voxels/Block.h"
#include "LevelFrontend.h"
#include "../items/Item.h"
#include "../graphics/Atlas.h"

InventoryView::InventoryView(int columns, const Content* content, std::vector<itemid_t> items, LevelFrontend *levelFrontend) : levelFrontend(levelFrontend), content(content), indices(content->indices), items(items), columns(columns) {
}

InventoryView::~InventoryView() {
}

void InventoryView::setPosition(int x, int y) {
    position.x = x;
    position.y = y;
}

int InventoryView::getWidth() const {
    return columns * iconSize + (columns - 1) * interval + padding.x * 2;
}

int InventoryView::getHeight() const {
    uint inv_rows = ceildiv(items.size(), columns);
    return inv_rows * iconSize + (inv_rows - 1) * interval + padding.y * 2;
}

void InventoryView::setSlotConsumer(slotconsumer consumer) {
    this->consumer = consumer;
}

void InventoryView::setItems(std::vector<itemid_t> items) {
    this->items = items;
}

void InventoryView::activateAndDraw(const GfxContext* ctx) {
    Assets* assets = levelFrontend->getAssets();
    ShaderProgram* uiShader = assets->getShader("ui");

    auto viewport = ctx->getViewport();
	uint inv_w = getWidth();
	uint inv_h = getHeight();
	int xs = position.x + padding.x;
	int ys = position.y + padding.y;

	glm::vec4 tint(1.0f);
	int mx = Events::cursor.x;
	int my = Events::cursor.y;

    auto batch = ctx->getBatch2D();
	batch->texture(nullptr);
	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
	batch->rect(position.x, position.y, inv_w, inv_h);
	batch->render();

    if (Events::scroll) scroll -= Events::scroll * (iconSize + interval);

    scroll = std::min(scroll, int(inv_h - viewport.getHeight()));
    scroll = std::max(scroll, 0);

    auto blocksPreview = levelFrontend->getBlocksPreview();
	{
		Window::clearDepth();
		GfxContext subctx = ctx->sub();
		subctx.depthTest(true);
		subctx.cullFace(true);
        uint index = 0;
		for (uint i = 0; i < items.size(); ++i) {
			Item* chosen_item = indices->getItemDef(items[i]);
			int x = xs + (iconSize + interval) * (index % columns);
			int y = ys + (iconSize + interval) * (index / columns) - scroll;
            if (y < -int(iconSize + interval) || y >= int(viewport.getHeight())) {
                ++index;
                continue;
            }
			if (mx > x && mx < x + (int)iconSize && my > y && my < y + (int)iconSize) {
				tint.r *= 1.2f;
				tint.g *= 1.2f;
				tint.b *= 1.2f;
				if (Events::justClicked(mousecode::BUTTON_1) && consumer) consumer(items[i]);
			} else {
				tint = glm::vec4(1.0f);
			}
			switch (chosen_item->iconType) {
				case ItemIconType::None:
					break;
                case ItemIconType::Block: {
                    Block* chosen_block = content->requireBlock(chosen_item->icon);
					blocksPreview->begin(&ctx->getViewport());
                    blocksPreview->draw(chosen_block, x, y, iconSize, tint);
                    break;
                }
				case ItemIconType::Sprite: {
					batch->begin();
                    uiShader->use();
                    size_t index = chosen_item->icon.find(':');
                    std::string name = chosen_item->icon.substr(index + 1);
                    UVRegion region(0.0f, 0.0, 1.0f, 1.0f);
                    if (index == std::string::npos) {
                        batch->texture(assets->getTexture(name));
                    } else {
                        std::string atlasname = chosen_item->icon.substr(0, index);
                        Atlas* atlas = assets->getAtlas(atlasname);
                        if (atlas && atlas->has(name)) {
                            region = atlas->get(name);
                            batch->texture(atlas->getTexture());
                        }
                    }
                    batch->rect(x, y, 48, 48, 0, 0, 0, region, false, true, glm::vec4(1.0f));
                    batch->render();
                    break;
				}
            }
            ++index;
		}
	}
	uiShader->use();
}
