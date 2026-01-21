#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <string>

#include "typedefs.h"

struct DisplaySettings {
	int width = 1280;
	int height = 720;
	int samples = 0;
	int swapInterval = 1;
	const char* title = "ChromaForge";
};

struct ChunksSettings {
	uint loadSpeed = 10;
	uint loadDistance = 22;
	uint padding = 2;
};

struct CameraSettings {
    bool fovEvents = true;
    bool shaking = true;
};

struct EngineSettings {
    DisplaySettings display;
	ChunksSettings chunks;
	CameraSettings camera;

	float fogCurve = 1.6f;
};

void read_settings(EngineSettings& settings, std::string filename);
void write_settings(EngineSettings& settings, std::string filename);

#endif // SRC_SETTINGS_H_
