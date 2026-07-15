#include <frontend/Panorama.h>

#include <GL/glew.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <graphics/core/Cubemap.h>
#include <graphics/core/Texture.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Mesh.h>
#include <graphics/core/gl_util.h>
#include <window/Camera.h>
#include <assets/Assets.h>
#include <debug/Logger.h>

std::unique_ptr<Cubemap> Panorama::loadCubemap(Assets& assets) {
    const char* faceNames[6] = {
        "0",  // GL_TEXTURE_CUBE_MAP_POSITIVE_X
        "1",   // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        "2",    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        "3", // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        "4",  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        "5"    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    Texture* faces[6] = {};
    uint width = 0, height = 0;

    for (int i = 0; i < 6; ++i) {
        faces[i] = assets.get<Texture>(std::string("panorama/") + faceNames[i]);
        if (!faces[i]) {
            LOG_ERROR("Panorama: missing texture {}", faceNames[i]);
            return nullptr;
            continue;
        }
        if (i == 0) {
            width = faces[i]->getWidth();
            height = faces[i]->getHeight();
        } else if (faces[i]->getWidth() != width || faces[i]->getHeight() != height) {
            LOG_ERROR("Panorama: texture sizes mismatch for face {}", faceNames[i]);
            return nullptr;
        }
    }

    // Создаём пустой кубмап нужного размера
    auto cubemap = std::make_unique<Cubemap>(width, height, ImageFormat::rgba8888);
    cubemap->bind();

    for (int i = 0; i < 6; ++i) {
        auto image = faces[i]->readData();
        image->flipY();

        GLenum format = gl::to_glenum(image->getFormat());
        glTexSubImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            0, 0,
            width, height,
            format, GL_UNSIGNED_BYTE,
            image->getData()
        );
    }

    cubemap->unbind();
    return cubemap;
}

struct ScreenQuadVertex {
    float x, y;

    static constexpr VertexAttribute ATTRIBUTES[] = {
        {VertexAttribute::Type::FLOAT, false, 2},
        {VertexAttribute::Type::FLOAT, false, 0}
    };
};

std::unique_ptr<Mesh<ScreenQuadVertex>> Panorama::createScreenQuad() {
    const ScreenQuadVertex vertices[] = {
        {-1.0f, -1.0f},
        { 3.0f, -1.0f},
        {-1.0f,  3.0f}
    };
    return std::make_unique<Mesh<ScreenQuadVertex>>(vertices, 3);
}

Panorama::Panorama(Assets& assets)
    : cubemap(loadCubemap(assets))
{
    if (cubemap) {
        camera = std::make_unique<Camera>(glm::vec3(0.0f), 90.0f);
        camera->perspective = true;
        camera->near = 0.1f;
        camera->far = 100.0f;

        mesh = createScreenQuad();
    } else {
        LOG_WARN("Cubemap not loaded, panorama disabled");
    }
}

Panorama::~Panorama() = default;

void Panorama::update(float deltaTime) {
    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > glm::two_pi<float>()) {
        rotationAngle -= glm::two_pi<float>();
    }
}

void Panorama::draw(ShaderProgram& shader, uint width, uint height) const {
    if (!cubemap || !mesh) return;

    camera->setAspectRatio(static_cast<float>(width) / height);

    camera->rotation = glm::mat4_cast(
        glm::angleAxis(rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f))
    );
    camera->updateVectors();
    camera->position = glm::vec3(0.0f);

    shader.use();
    shader.uniformMatrix("u_view", camera->getView());
    shader.uniform1f("u_ar", static_cast<float>(width) / height);
    shader.uniform1f("u_zoom", 1.0f);

    glActiveTexture(GL_TEXTURE0);
    cubemap->bind();
    shader.uniform1i("u_skybox", 0);

    mesh->draw();

    cubemap->unbind();
}
