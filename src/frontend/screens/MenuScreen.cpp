#include <frontend/screens/MenuScreen.h>

#include <graphics/ui/GUI.h>
#include <graphics/ui/elements/layout/Menu.h>
#include <graphics/core/Batch2D.h>
#include <graphics/core/ShaderProgram.h>
#include <window/Window.h>
#include <window/Camera.h>
#include <engine/Engine.h>
#include <graphics/core/Texture.h>
#include <math/UVRegion.h>
#include <graphics/core/Cubemap.h>
#include <debug/Logger.h>
#include <graphics/core/Mesh.h>
#include <graphics/core/gl_util.h>
#include <assets/Assets.h>

std::unique_ptr<Cubemap> MenuScreen::loadPanoramaCubemap(Assets& assets) {
    constexpr int FACES = 6;
    Texture* textures[FACES] = {};
    std::unique_ptr<ImageData> images[FACES];

    for (int i = 0; i < FACES; ++i) {
        std::string name = "panorama/" + std::to_string(i);
        textures[i] = assets.get<Texture>(name);
        if (!textures[i]) {
            LOG_WARN("Missing panorama texture: {}", name);
            return nullptr;
        }
    }

    uint width  = textures[0]->getWidth();
    uint height = textures[0]->getHeight();
    for (int i = 1; i < FACES; ++i) {
        if (textures[i]->getWidth() != width || textures[i]->getHeight() != height) {
            LOG_ERROR("Panorama texture size mismatch");
            return nullptr;
        }
    }

    for (int i = 0; i < FACES; ++i) {
        images[i] = textures[i]->readData();
        if (!images[i]) {
            LOG_ERROR("Failed to read image data from panorama texture {}", i);
            return nullptr;
        }
        if (images[i]->getFormat() != ImageFormat::rgba8888) {
            LOG_WARN("Panorama texture {} is not RGBA", i);
        }
        images[i]->flipY();
    }

    auto cubemap = std::make_unique<Cubemap>(width, height, ImageFormat::rgba8888);
    cubemap->bind();

    GLenum glformat = gl::to_glenum(ImageFormat::rgba8888);
    for (int i = 0; i < FACES; ++i) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            glformat,
            width, height, 0,
            glformat,
            GL_UNSIGNED_BYTE,
            images[i]->getData()
        );
    }
    cubemap->unbind();
    return cubemap;
}

MenuScreen::MenuScreen(Engine& engine) : Screen(engine) {
    engine.resetContent();

    auto menu = engine.getGUI()->getMenu();
    menu->reset();
    menu->setPage("main");

    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);
    uicamera->perspective = false;
    uicamera->flipped = true;

    try {
        panoramaCubemap = loadPanoramaCubemap(*engine.getAssets());
        if (panoramaCubemap) {
            panoramaCamera = std::make_unique<Camera>(glm::vec3(), glm::radians(70.0f));
            panoramaCamera->perspective = true;

            float vertices[] = {
                -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
                -1.0f, -1.0f,  1.0f, 1.0f, 1.0f, -1.0f
            };
            VertexAttribute attrs[] = {{2}, {0}};
            panoramaMesh = std::make_unique<Mesh>(vertices, 6, attrs);

            hasPanorama = true;
            LOG_INFO("Main menu panorama loaded successfully");
        }
    } catch (const std::exception& e) {
        LOG_WARN("Failed to load panorama: {}", e.what());
    }
}

MenuScreen::~MenuScreen() = default;

void MenuScreen::update(float deltaTime) {
    if (!hasPanorama) return;

    constexpr float SPEED = 0.1f;
    rotationAngle += SPEED * deltaTime;

    panoramaCamera->rotation = glm::mat4(1.0f);
    panoramaCamera->rotate(0.0f, rotationAngle, 0.0f);

    float tilt = glm::sin(rotationAngle * 0.7f) * 0.05f;
    panoramaCamera->rotate(tilt, 0.0f, 0.0f);
    panoramaCamera->updateVectors();
}

void MenuScreen::draw(float deltaTime) {
    auto assets = engine.getAssets();
    uint width = Window::width;
    uint height = Window::height;

    Window::clear();

    if (hasPanorama) {
        auto backShader = assets->get<ShaderProgram>("background");
        backShader->use();
        backShader->uniformMatrix("u_view", panoramaCamera->getView(false));
        backShader->uniform1f("u_zoom", panoramaCamera->zoom * panoramaCamera->getFov() / (PI * 0.5f));
        backShader->uniform1f("u_ar", float(width) / float(height));
        backShader->uniform1i("u_cubemap", 1);

        glActiveTexture(GL_TEXTURE1);
        panoramaCubemap->bind();
        glActiveTexture(GL_TEXTURE0);

        panoramaMesh->draw();

        glActiveTexture(GL_TEXTURE1);
        panoramaCubemap->unbind();
        glActiveTexture(GL_TEXTURE0);
    } else {
        Window::setBgColor(glm::vec3(0.2f));

        uicamera->setFov(Window::height);

        ShaderProgram* uishader = assets->get<ShaderProgram>("ui");
        uishader->use();
        uishader->uniformMatrix("u_projview", uicamera->getProjView());

        auto bg = assets->get<Texture>("gui/menubg");
        batch->begin();
        batch->texture(bg);
        batch->rect(
            0, 0,
            width, height, 0, 0, 0,
            UVRegion(0, 0, width / bg->getWidth(), height / bg->getHeight()),
            false, false, glm::vec4(1.0f)
        );
        batch->flush();
    }
}
