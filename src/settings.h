#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include "typedefs.h"

struct DisplaySettings {
	int width = 1280;
	int height = 720;
	int samples = 0;
	int swapInterval = 1;
	const char* title = "ChromaForge";

	bool fullscreen = false;
};

struct ChunksSettings {
	uint loadSpeed = 10;
	uint loadDistance = 22;
	uint padding = 2;

	bool occlusion = true;
};

struct CameraSettings {
    bool fovEvents = true;
    bool shaking = true;
};

struct GraphicsSettings {
    float fogCurve = 1.6f;
	bool backlight = true;
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
