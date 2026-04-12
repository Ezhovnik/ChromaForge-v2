#ifndef LOGIC_PLAYERCONTROLLER_H_
#define LOGIC_PLAYERCONTROLLER_H_

#include <memory>
#include <vector>
#include <functional>

#include <glm/glm.hpp>

#include "../objects/Player.h"
#include "../interfaces/Object.h"

class Camera;
class BlocksController;
class Level;
class Chunks;
class Block;
struct EngineSettings;
struct CameraSettings;
struct Hitbox;

class CameraControl {
private:
	std::shared_ptr<Player> player;
    std::shared_ptr<Camera> camera;
    const CameraSettings& settings;
    glm::vec3 offset;
    float shake = 0.0f;
    float shakeTimer = 0.0f;
    glm::vec3 interpVel {0.0f};

	glm::vec3 updateCameraShaking(const Hitbox& hitbox, float delta);

	void updateFovEffects(const Hitbox& hitbox, const PlayerInput& input, float delta);

	void switchCamera();
public:
	CameraControl(
		const std::shared_ptr<Player>& player,
		const CameraSettings& settings
	);

	void updateMouse(PlayerInput& input);
	void update(const PlayerInput& input, float delta, Chunks* chunks);
	void refresh();
};

enum class BlockInteraction {
    Step,
    Destruction,
    Placing
};

using on_block_interaction = std::function<void(Player*, glm::ivec3, const Block*, BlockInteraction type)>;

class PlayerController {
private:
	Level* level;
	std::shared_ptr<Player> player;
	PlayerInput input {};
	CameraControl camControl;
	BlocksController* blocksController;

	std::vector<on_block_interaction> blockInteractionCallbacks;

	void updateKeyboard();
	void updateCamera(float delta, bool movement);
	void resetKeyboard();
	void updatePlayer(float deltaTime);
	void updateInteraction();
	void onBlockInteraction(
        glm::ivec3 pos,
        const Block* def,
        BlockInteraction type
    );

    float stepsTimer = 0.0f;
    void onFootstep(const Hitbox& hitbox);
    void updateFootsteps(float delta);

	void processRightClick(Block* def, Block* target);

	voxel* updateSelection(float maxDistance);
public:

	PlayerController(Level* level, const EngineSettings& settings, BlocksController* blocksController);

	void update(float delta, bool input_flag, bool pause);
	void postUpdate(float delta, bool input_flag, bool pause);

	Player* getPlayer();

	void listenBlockInteraction(const on_block_interaction& callback);
};

#endif // LOGIC_PLAYERCONTROLLER_H_
