#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

class Batch2D;
class Camera;
class Level;
class Assets;
class Player;

class HudRenderer {
	Batch2D* batch;
	Camera* uicamera;
    Assets* assets;
public:
	HudRenderer(Assets* assets);
	~HudRenderer();
	void draw(Level* level);
	void drawDebug(Level* level, int fps, bool occlusion);
    void drawInventory(Player* player);
};

#endif // SRC_HUD_RENDER_H_
