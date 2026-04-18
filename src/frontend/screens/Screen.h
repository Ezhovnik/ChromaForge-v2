#ifndef FRONTEND_SCREENS_SCREEN_H_
#define FRONTEND_SCREENS_SCREEN_H_

#include "util/ObjectsKeeper.h"

class Engine;
class Batch2D;

class Screen : public util::ObjectsKeeper {
protected:
    Engine* engine;
    std::unique_ptr<Batch2D> batch;
public:
    Screen(Engine* engine);
    virtual ~Screen();
    virtual void update(float deltaTime) = 0;
    virtual void draw(float deltaTime) = 0;
    virtual void onEngineShutdown() {};
};

#endif // FRONTEND_SCREENS_SCREEN_H_
