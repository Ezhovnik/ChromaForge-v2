#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "../graphics/GfxContext.h"

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;
class Engine;
class ContentGfxCache;
class WorldRenderer;
class Block;
class BlocksPreview;

namespace gui {
    class GUI;
    class UINode;
}

class HudRenderer {
private:
    Level* level;
	Batch2D* batch;
	Camera* uicamera;
    Assets* assets;
    BlocksPreview* blocksPreview;

    int fps = 0;
    int fpsMin = 60;
    int fpsMax = 60;
    std::wstring fpsString;

    bool inventoryOpen = false;
    bool pause = false;

    std::shared_ptr<gui::UINode> debugPanel;
	gui::GUI* guiController;
    const ContentGfxCache* const cache;
public:
	HudRenderer(Engine* engine, Level* level, const ContentGfxCache* cache);
	~HudRenderer();

    void update();
	void draw(const GfxContext& context);
	void drawDebug(int fps);
    void drawContentAccess(const GfxContext& context, Player* player);

    bool isInventoryOpen() const;
	bool isPause() const;
};

#endif // SRC_HUD_RENDER_H_
