#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

class Batch2D;
class Camera;
class Level;
class Assets;
class Mesh;

namespace Crosshair_Consts {
    const float VERTICES[] = {
        -0.01f,-0.01f,
		0.01f, 0.01f,

		-0.01f, 0.01f,
		0.01f,-0.01f,
	};

    const int ATTRS[] = {2, 0};
}

class HudRenderer {
	Batch2D* batch;
	Camera* uicamera;
	Mesh* crosshair;
public:
	HudRenderer();
	~HudRenderer();
	void draw(Level* level, Assets* assets);
	void drawDebug(Level* level, Assets* assets, int fps, bool occlusion);
};

#endif // SRC_HUD_RENDER_H_
