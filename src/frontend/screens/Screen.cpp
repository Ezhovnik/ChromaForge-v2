#include <frontend/screens/Screen.h>

#include <graphics/core/Batch2D.h>
#include <engine.h>

Screen::Screen(Engine* engine) : engine(engine), batch(std::make_unique<Batch2D>(1024)) {
}

Screen::~Screen() {
}
