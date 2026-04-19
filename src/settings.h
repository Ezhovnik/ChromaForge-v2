#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <string>

#include <typedefs.h>
#include <constants.h>
#include "data/setting.h"

struct DisplaySettings {
	IntegerSetting width {1280};
	IntegerSetting height {720};
	IntegerSetting samples {0};
	IntegerSetting framerate{-1, -1, 120};

	BoolSetting fullscreen {false};
};

struct ChunksSettings {
	IntegerSetting loadSpeed {4, 1, 32};
	IntegerSetting loadDistance {22, 3, 80};
	IntegerSetting padding {2, 1, 8};
};

struct CameraSettings {
    BoolSetting fovEffects {true};
    BoolSetting shaking {true};
	BoolSetting inertia {true};
	NumberSetting fov {90.0f, 10, 120};
	NumberSetting sensitivity {2.0f, 0.1f, 10.0f};
};

struct GraphicsSettings {
    NumberSetting fogCurve {1.0f, 1.0f, 6.0f};
	NumberSetting gamma {1.0f, 0.4f, 1.0f};
	BoolSetting backlight {true};
	BoolSetting frustumCulling {true};
	IntegerSetting skyboxResolution {64 + 32, 64, 128};
};

struct DebugSettings {
    BoolSetting generatorTestMode {false};
	BoolSetting doWriteLights {true};
};

struct UISettings {
	StringSetting language {"auto"};

	IntegerSetting worldPreviewSize {64, 1, 512};
};

struct AudioSettings {
    BoolSetting enabled {true};

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
