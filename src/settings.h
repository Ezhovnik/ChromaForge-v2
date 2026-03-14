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
	std::string title = "ChromaForge (v " + ENGINE_VERSION_STRING + ")";

	bool fullscreen = false;
};

struct ChunksSettings {
	uint loadSpeed = 4;
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
	float gamma = 1.0f;
	bool backlight = true;
	bool frustumCulling = true;
	int skyboxResolution = 64 + 32;
};

struct DebugSettings {
    bool generatorTestMode = false;
	bool doWriteLights = true;
};

struct UISettings {
	std::string language = "auto";
};

struct AudioSettings {
    bool enabled = true;

    float volumeMaster = 1.0f;
    float volumeRegular = 1.0f;
    float volumeUI = 1.0f;
    float volumeAmbient = 1.0f;
    float volumeMusic = 1.0f;
};

struct EngineSettings {
	AudioSettings audio;
    DisplaySettings display;
	ChunksSettings chunks;
    CameraSettings camera;
    GraphicsSettings graphics;
    DebugSettings debug;
	UISettings ui;
};

#endif // SRC_SETTINGS_H_
