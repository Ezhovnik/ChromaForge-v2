#include <frontend/Panorama.h>

#include <GL/glew.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <graphics/core/Cubemap.h>
#include <graphics/core/Texture.h>
#include <graphics/core/ImageData.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Mesh.h>
#include <graphics/core/gl_util.h>
#include <window/Camera.h>
#include <assets/Assets.h>
#include <debug/Logger.h>

static debug::Logger logger("panorama");

static const std::array<std::string, 6> DEFAULT_FACES = {
    "panorama/0",
    "panorama/1",
    "panorama/2",
    "panorama/3",
    "panorama/4",
    "panorama/5"
};

std::unique_ptr<Cubemap> Panorama::loadCubemap(
    Assets& assets, const std::array<std::string, 6>& faces
) {
    Texture* loadedFaces[6] = {};
    uint width = 0, height = 0;

    for (int i = 0; i < 6; ++i) {
        loadedFaces[i] = assets.get<Texture>(faces[i]);
        if (!loadedFaces[i]) {
            logger.error() << "Missing texture " << faces[i];
            return nullptr;
        }
        if (i == 0) {
            width = loadedFaces[i]->getWidth();
            height = loadedFaces[i]->getHeight();
        } else if (
            loadedFaces[i]->getWidth() != width
            || loadedFaces[i]->getHeight() != height
        ) {
            logger.error() << "Texture sizes mismatch for face " << faces[i];
            return nullptr;
        }
    }

    // Создаём пустой кубмап нужного размера
    auto cubemap = std::make_unique<Cubemap>(width, height, ImageFormat::rgba8888);
    cubemap->bind();

    for (int i = 0; i < 6; ++i) {
        auto image = loadedFaces[i]->readData();
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

Panorama::Panorama(Assets& assets) : assets(assets) {
    camera = std::make_unique<Camera>(glm::vec3(0.0f), 90.0f);
    camera->perspective = true;
    camera->near = 0.1f;
    camera->far = 100.0f;

    mesh = createScreenQuad();
    setTextures(DEFAULT_FACES);
}

bool Panorama::setTextures(const std::array<std::string, 6>& faces) {
    if (auto loaded = loadCubemap(assets, faces)) {
        cubemap = std::move(loaded);
        return true;
    }
    logger.warning() << "Cubemap not loaded, panorama disabled";
    return false;
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

float Panorama::getRotationSpeed() const {
    return rotationSpeed;
}
void Panorama::setRotationSpeed(float speed) {
    rotationSpeed = speed;
}

float Panorama::getRotation() const {
    return glm::degrees(rotationAngle);
}
void Panorama::setRotation(float angle) {
    rotationAngle = glm::radians(angle);
}
