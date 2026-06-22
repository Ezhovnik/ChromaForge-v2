#include <frontend/Panorama.h>

#include <assets/Assets.h>
#include <graphics/core/Cubemap.h>
#include <graphics/core/Mesh.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/ImageData.h>
#include <graphics/core/gl_util.h>
#include <window/Camera.h>
#include <debug/Logger.h>
#include <constants.h>

std::unique_ptr<Cubemap> Panorama::loadCubemap(Assets& assets) {
    constexpr int FACES = 6;
    Texture* textures[FACES] = {};

    for (int i = 0; i < FACES; ++i) {
        textures[i] = assets.get<Texture>("panorama/" + std::to_string(i));
        if (!textures[i]) return nullptr;
    }

    uint width = textures[0]->getWidth();
    uint height = textures[0]->getHeight();
    for (int i = 1; i < FACES; ++i) {
        if (textures[i]->getWidth() != width || textures[i]->getHeight() != height) {
            LOG_ERROR("Panorama texture size mismatch");
            return nullptr;
        }
    }

    auto cubemap = std::make_unique<Cubemap>(width, height, ImageFormat::rgba8888);
    cubemap->bind();
    GLenum glformat = gl::to_glenum(ImageFormat::rgba8888);

    for (int i = 0; i < FACES; ++i) {
        auto image = textures[i]->readData();
        if (!image) {
            LOG_ERROR("Failed to read image data from panorama texture {}", i);
            cubemap->unbind();
            return nullptr;
        }
        if (image->getFormat() != ImageFormat::rgba8888) {
            LOG_WARN("Panorama texture {} is not RGBA", i);
        }
        image->flipY();

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glformat,
            width, height, 0, glformat, GL_UNSIGNED_BYTE, image->getData()
        );
    }
    cubemap->unbind();
    return cubemap;
}

std::unique_ptr<Mesh> Panorama::createScreenQuad() {
    float vertices[] = {
        -1.0f, -1.0f,  -1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f, -1.0f,   1.0f,  1.0f,   1.0f, -1.0f
    };
    VertexAttribute attrs[] = {{2}, {0}};
    return std::make_unique<Mesh>(vertices, 6, attrs);
}

Panorama::Panorama(Assets& assets) {
    cubemap = loadCubemap(assets);
    if (!cubemap) return;

    camera = std::make_unique<Camera>(glm::vec3(0.0f), glm::radians(70.0f));
    camera->perspective = true;
    mesh = createScreenQuad();
}

Panorama::~Panorama() = default;

bool Panorama::isValid() const {
    return cubemap != nullptr;
}

void Panorama::update(float deltaTime) {
    if (!isValid()) return;

    constexpr float SPEED = 0.1f;
    rotationAngle += SPEED * deltaTime;

    camera->rotation = glm::mat4(1.0f);
    camera->rotate(0.0f, rotationAngle, 0.0f);

    float tilt = glm::sin(rotationAngle * 0.7f) * 0.05f;
    camera->rotate(tilt, 0.0f, 0.0f);
    camera->updateVectors();
}

void Panorama::draw(ShaderProgram& shader, uint width, uint height) const {
    if (!isValid()) return;

    shader.use();
    shader.uniformMatrix("u_view", camera->getView(false));
    shader.uniform1f("u_zoom", camera->zoom * camera->getFov() / (PI * 0.5f));
    shader.uniform1f("u_ar", float(width) / float(height));
    shader.uniform1i("u_cubemap", 1);

    glActiveTexture(GL_TEXTURE1);
    cubemap->bind();
    glActiveTexture(GL_TEXTURE0);

    mesh->draw();

    glActiveTexture(GL_TEXTURE1);
    cubemap->unbind();
    glActiveTexture(GL_TEXTURE0);
}
