#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <string>

#include "typedefs.h"
#include "constants.h"

struct DisplaySettings {
	int width = 1280;
	int height = 720;
	int samples = 0;
	int swapInterval = 1;
	std::string title = "ChromaForge (v " + 
		std::to_string(ENGINE_VERSION_MAJOR) + "." + 
		std::to_string(ENGINE_VERSION_MINOR) + "." + 
		std::to_string(ENGINE_VERSION_MAINTENANCE) + ")";

	bool fullscreen = false;
};

struct ChunksSettings {
	uint loadSpeed = 10;
	uint loadDistance = 22;
	uint padding = 2;
};

struct CameraSettings {
    bool fovEvents = true;
    bool shaking = true;
	float fov = 90.0f;
	float sensitivity = 2.0f;
};

struct GraphicsSettings {
    float fogCurve = 1.6f;
	bool backlight = true;
	bool frustumCulling = true;
	int skyboxResolution = 64 + 32;
};

struct DebugSettings {
    bool generatorTestMode = false;
};

struct EngineSettings {
    DisplaySettings display;
	ChunksSettings chunks;
    CameraSettings camera;
    GraphicsSettings graphics;
    DebugSettings debug;
};

#endif // SRC_SETTINGS_H_
