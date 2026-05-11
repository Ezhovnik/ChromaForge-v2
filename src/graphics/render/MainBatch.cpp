#include <graphics/render/MainBatch.h>

#include <graphics/core/Texture.h>
#include <graphics/core/Mesh.h>
#include <graphics/core/ImageData.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>

static const vattr attrs[] = {
    {3}, {2}, {3}, {1}, {0}
};

MainBatch::MainBatch(
    size_t capacity
) : buffer(std::make_unique<float[]>(capacity * VERTEX_SIZE)),
    capacity(capacity),
    index(0),
    mesh(std::make_unique<Mesh>(buffer.get(), 0, attrs)) 
{
    const ubyte pixels[] = {
        255, 255, 255, 255,
    };
    ImageData image(ImageFormat::rgba8888, 1, 1, pixels);
    blank = Texture::from(&image);
}

MainBatch::~MainBatch() = default;

void MainBatch::setTexture(const Texture* texture) {
    if (texture == nullptr) {
        texture = blank.get();
    }
    if (texture != this->texture) {
        flush();
    }
    this->texture = texture;
    region = UVRegion {0.0f, 0.0f, 1.0f, 1.0f};
}

void MainBatch::setTexture(const Texture* texture, const UVRegion& region) {
    setTexture(texture);
    this->region = region;
}

void MainBatch::flush() {
    if (index == 0) return;

    if (texture == nullptr) {
        texture = blank.get();
    }
    texture->bind();
    mesh->reload(buffer.get(), index / VERTEX_SIZE);
    mesh->draw();
    index = 0;
}

void MainBatch::begin() {
    texture = nullptr;
    blank->bind();
}

void MainBatch::prepare(int vertices) {
    if (index + VERTEX_SIZE * vertices > capacity * VERTEX_SIZE) {
        flush();
    }
}

glm::vec4 MainBatch::sampleLight(
    const glm::vec3& pos, const Chunks& chunks, bool backlight
) {
    light_t light = chunks.getLight(
        std::floor(pos.x), 
        std::floor(std::min(CHUNK_HEIGHT - 1.0f, pos.y)), 
        std::floor(pos.z)
    );
    light_t minIntensity = backlight ? 1 : 0;
    return glm::vec4(
        glm::max(LightMap::extract(light, 0), minIntensity) / 15.0f,
        glm::max(LightMap::extract(light, 1), minIntensity) / 15.0f,
        glm::max(LightMap::extract(light, 2), minIntensity) / 15.0f,
        glm::max(LightMap::extract(light, 3), minIntensity) / 15.0f
    );
}
