#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

#include <string>

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;

namespace gui {
    class GUI;
    class Panel;
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
    bool pause = true;

    gui::Panel* debugPanel;
	gui::Panel* pauseMenu;
	gui::GUI* guiController;
public:
	HudRenderer(gui::GUI* gui, Level* level, Assets* assets);
	~HudRenderer();

	void draw();
	void drawDebug(int fps, bool occlusion);
    void drawInventory(Player* player);

    bool isInventoryOpen() const;
	bool isPause() const;
};

#endif // SRC_HUD_RENDER_H_
