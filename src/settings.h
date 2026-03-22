#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <string>

#include "typedefs.h"
#include "constants.h"
#include "data/setting.h"

struct DisplaySettings {
	int width = 1280;
	int height = 720;
	int samples = 0;
	BoolSetting vsync = {true};
	std::string title = "ChromaForge (v " + ENGINE_VERSION_STRING + ")";

	bool fullscreen = false;
};

struct ChunksSettings {
	IntegerSetting loadSpeed = {4, 1, 32};
	IntegerSetting loadDistance = {22, 3, 66};
	IntegerSetting padding = {2, 1, 8};
};

struct CameraSettings {
    bool fovEvents = true;
    BoolSetting shaking = {true};
	NumberSetting fov {90.0f, 10, 120};
	NumberSetting sensitivity {2.0f, 0.1f, 10.0f};
};

struct GraphicsSettings {
    NumberSetting fogCurve {1.6f, 1.0f, 6.0f};
	float gamma = 1.0f;
	BoolSetting backlight = {true};
	bool frustumCulling = true;
	int skyboxResolution = 64 + 32;
};

struct DebugSettings {
    bool generatorTestMode = false;
	bool doWriteLights = true;
};

struct UISettings {
	std::string language = "auto";

	IntegerSetting worldPreviewSize {64, 1, 512};
};

struct AudioSettings {
    bool enabled = true;

    NumberSetting volumeMaster {1.0f, 0.0f, 1.0f, SettingFormat::Percent};
    NumberSetting volumeRegular {1.0f, 0.0f, 1.0f, SettingFormat::Percent};
    NumberSetting volumeUI {1.0f, 0.0f, 1.0f, SettingFormat::Percent};
    NumberSetting volumeAmbient {1.0f, 0.0f, 1.0f, SettingFormat::Percent};
    NumberSetting volumeMusic {1.0f, 0.0f, 1.0f, SettingFormat::Percent};
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
