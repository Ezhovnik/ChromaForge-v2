#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

#include <string>
#include <memory>

#include "../graphics/GfxContext.h"

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;
class Engine;

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

    int fps = 0;
    int fpsMin = 60;
    int fpsMax = 60;
    std::wstring fpsString;

    bool occlusion;
    bool inventoryOpen = false;
    bool pause = false;

    std::shared_ptr<gui::UINode> debugPanel;
	std::shared_ptr<gui::UINode> pauseMenu;
	gui::GUI* guiController;
public:
	HudRenderer(Engine* engine, Level* level);
	~HudRenderer();

	void draw(const GfxContext& context);
	void drawDebug(int fps, bool occlusion);
    void drawInventory(const GfxContext& context, Player* player);

    bool isInventoryOpen() const;
	bool isPause() const;
};

#endif // SRC_HUD_RENDER_H_
