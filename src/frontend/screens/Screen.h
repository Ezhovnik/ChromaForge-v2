#pragma once

#include <util/ObjectsKeeper.h>

class Engine;
class Batch2D;

class Screen : public util::ObjectsKeeper {
protected:
    Engine& engine;
    std::unique_ptr<Batch2D> batch;
public:
    Screen(Engine& engine);
    virtual ~Screen();
    virtual void onOpen() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void draw(float deltaTime) = 0;
    virtual void onEngineShutdown() {};
    virtual const char* getName() const = 0;
};
