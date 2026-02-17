#include "BlocksPreview.h"

#include <glm/ext.hpp>

#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../graphics/Atlas.h"
#include "../graphics/Batch3D.h"
#include "../window/Camera.h"
#include "../voxels/Block.h"
#include "../window/Window.h"
#include "ContentGfxCache.h"

BlocksPreview::BlocksPreview(ShaderProgram* shader, Atlas* atlas, const ContentGfxCache* cache) : shader(shader), atlas(atlas), cache(cache) {
    batch = new Batch3D(1024);
}

BlocksPreview::~BlocksPreview() {
    delete batch;
}

void BlocksPreview::begin() {
    shader->use();
    shader->uniformMatrix("u_projview", 
        glm::ortho(0.0f, float(Window::width), 0.0f, float(Window::height), -1000.0f, 1000.0f) * 
        glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0.0f), glm::vec3(0, 1, 0))
    );
    atlas->getTexture()->bind();
}

void BlocksPreview::draw(const Block* def, int x, int y, int size, glm::vec4 tint) {
    y = Window::height - y - 1;
    x += 2;
    y -= 35;
    shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), glm::vec3(x / float(Window::width) * 2, y / float(Window::height) * 2, 0.0f)));
    blockid_t id = def->rt.id;
    const UVRegion texfaces[6]{ cache->getRegion(id, 0), cache->getRegion(id, 1),
                                cache->getRegion(id, 2), cache->getRegion(id, 3),
                                cache->getRegion(id, 4), cache->getRegion(id, 5)
                            };

    switch (def->model) {
        case BlockModel::None:
            break;
        case BlockModel::Cube:
            batch->blockCube(glm::vec3(size * 0.63f), texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::AABB:
            batch->blockCube(def->hitbox.size() * glm::vec3(size * 0.63f), texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::X: {
            glm::vec3 right = glm::normalize(glm::vec3(1.f, 0.f, -1.f));
            batch->sprite(right * float(size) * 0.43f + glm::vec3(0, size * 0.4f, 0), glm::vec3(0.f, 1.f, 0.f), right, size * 0.5f, size * 0.6f, texfaces[0], tint);
            break;
        }
    }
    
    batch->flush();
}
