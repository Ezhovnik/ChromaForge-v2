#ifndef FRONTEND_SCREENS_MENU_SCREEN_H_
#define FRONTEND_SCREENS_MENU_SCREEN_H_

#include <memory>

#include <frontend/screens/Screen.h>

class Camera;
class Engine;

class MenuScreen : public Screen {
private:
    std::unique_ptr<Camera> uicamera;
public:
    MenuScreen(Engine* engine);
    ~MenuScreen();

    void update(float deltaTime) override;
    void draw(float deltaTime) override;
};


#endif // FRONTEND_SCREENS_MENU_SCREEN_H_
