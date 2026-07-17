#pragma once

#include <string>

#include <typedefs.h>
#include <constants.h>
#include <data/setting.h>

struct DisplaySettings {
	IntegerSetting width {1280};
	IntegerSetting height {720};
	IntegerSetting samples {0};
	IntegerSetting framerate{-1, -1, 120};

	BoolSetting fullscreen {false};

	BoolSetting limitFpsIconified {false};
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
	BoolSetting denseRender {true};
	BoolSetting frustumCulling {true};
	IntegerSetting skyboxResolution {64 + 32, 64, 128};
	IntegerSetting chunkMaxVertices {200'000, 0, 4'000'000};
	IntegerSetting chunkMaxVerticesDense {800'000, 0, 8'000'000};
    IntegerSetting chunkMaxRenderers {6, -4, 32};
	BoolSetting advancedRender {true};
	BoolSetting ssao {true};
	IntegerSetting shadowsQuality {0, 0, 3};
	IntegerSetting denseRenderDistance {56, 0, 10'000};
};

struct PathfindingSettings {
    IntegerSetting stepsPerAsyncAgent {256, 1, 2048};
};

struct DebugSettings {
    BoolSetting generatorTestMode {false};
	BoolSetting doWriteLights {true};
	BoolSetting doTraceShaders {false};
	BoolSetting enableExperimental {false};
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

struct NetworkSettings {
};

struct EngineSettings {
	AudioSettings audio;
    DisplaySettings display;
	ChunksSettings chunks;
    CameraSettings camera;
    GraphicsSettings graphics;
    DebugSettings debug;
	UISettings ui;
	NetworkSettings network;
	PathfindingSettings pathfinding;
};
