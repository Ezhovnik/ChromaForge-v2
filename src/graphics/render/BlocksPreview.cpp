#include "BlocksPreview.h"

#include <glm/ext.hpp>

#include "graphics/core/ShaderProgram.h"
#include "graphics/core/Texture.h"
#include "graphics/core/Atlas.h"
#include "graphics/core/Batch3D.h"
#include <window/Camera.h>
#include <voxels/Block.h>
#include "graphics/core/Viewport.h"
#include "frontend/ContentGfxCache.h"
#include <assets/Assets.h>
#include "graphics/core/Framebuffer.h"
#include "graphics/core/DrawContext.h"
#include <window/Window.h>
#include "content/Content.h"
#include <constants.h>
#include "graphics/core/ImageData.h"

std::unique_ptr<ImageData> BlocksPreview::draw(
    const ContentGfxCache* cache, 
    ShaderProgram* shader,
    Framebuffer* fbo, 
    Batch3D* batch, 
    const Block& def, 
    int size
) {
    Window::clear();
    blockid_t id = def.rt.id;
    const UVRegion texfaces[6]{
        cache->getRegion(id, 0), cache->getRegion(id, 1),
        cache->getRegion(id, 2), cache->getRegion(id, 3),
        cache->getRegion(id, 4), cache->getRegion(id, 5)
    };

    glm::vec3 offset(0.1f, 0.5f, 0.1f);
    switch (def.model) {
        case BlockModel::None:
            break;
        case BlockModel::Cube:
            shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
            batch->blockCube(glm::vec3(size * 0.63f), texfaces, glm::vec4(1.0f), !def.rt.emissive);
            batch->flush();
            break;
        case BlockModel::AABB:
            {
                glm::vec3 hitbox {};
                for (const auto& box : def.hitboxes) {
                    hitbox = glm::max(hitbox, box.size());
                }
                offset = glm::vec3(1, 1, 0.0f);
                shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
                glm::vec3 scaledSize = glm::vec3(size * 0.63f);
                batch->cube(
                    -hitbox * scaledSize * 0.5f * glm::vec3(1, 1, -1),
                    hitbox * scaledSize, 
                    texfaces, glm::vec4(1.0f), 
                    !def.rt.emissive
                );
            }
            batch->flush();
            break;
        case BlockModel::Custom:
            {
                glm::vec3 pmul = glm::vec3(size * 0.63f);
                glm::vec3 hitbox = glm::vec3();
                for (const auto& box : def.modelBoxes) {
                    hitbox = glm::max(hitbox, box.size());
                }
                offset.y += (1.0f - hitbox).y * 0.5f;
                shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
                for (size_t i = 0; def.modelBoxes.size(); ++i) {
                    const UVRegion (&boxtexfaces)[6] = {
                        def.modelUVs[i * 6],
                        def.modelUVs[i * 6 + 1],
                        def.modelUVs[i * 6 + 2],
                        def.modelUVs[i * 6 + 3],
                        def.modelUVs[i * 6 + 4],
                        def.modelUVs[i * 6 + 5]
                    };
                    batch->cube(
                        def.modelBoxes[i].a * glm::vec3(1.0f, 1.0f, -1.0f) * pmul, 
                        def.modelBoxes[i].size() * pmul, 
                        boxtexfaces, glm::vec4(1.0f), !def.rt.emissive
                    );
                }

                auto& points = def.modelExtraPoints;
                glm::vec3 poff = glm::vec3(0.0f, 0.0f, 1.0f);

                for (size_t i = 0; i < def.modelExtraPoints.size() / 4; ++i) {
                    const UVRegion& reg = def.modelUVs[def.modelBoxes.size() * 6 + i];
                    batch->point((points[i * 4 + 0] - poff) * pmul, glm::vec2(reg.u1, reg.v1), glm::vec4(1.0));
                    batch->point((points[i * 4 + 1] - poff) * pmul, glm::vec2(reg.u2, reg.v1), glm::vec4(1.0));
                    batch->point((points[i * 4 + 2] - poff) * pmul, glm::vec2(reg.u2, reg.v2), glm::vec4(1.0));
                    batch->point((points[i * 4 + 0] - poff) * pmul, glm::vec2(reg.u1, reg.v1), glm::vec4(1.0));
                    batch->point((points[i * 4 + 2] - poff) * pmul, glm::vec2(reg.u2, reg.v2), glm::vec4(1.0));
                    batch->point((points[i * 4 + 3] - poff) * pmul, glm::vec2(reg.u1, reg.v2), glm::vec4(1.0));
                }
                batch->flush();
            }
            break;
        case BlockModel::X: {
            glm::vec3 right = glm::normalize(glm::vec3(1.0f, 0.0f, -1.0f));
            batch->sprite(
                right * float(size) * 0.43f + glm::vec3(0, size * 0.4f, 0), 
                glm::vec3(0.0f, 1.0f, 0.0f), 
                right, 
                size * 0.5f, size * 0.6f, 
                texfaces[0], 
                glm::vec4(1.0f)
            );
            batch->flush();
            break;
        }
    }
    return fbo->getTexture()->readData();
}

std::unique_ptr<Atlas> BlocksPreview::build(const ContentGfxCache* cache, Assets* assets, const Content* content) {
    auto indices = content->getIndices();
    size_t count = indices->blocks.count();
    size_t iconSize = ITEM_ICON_SIZE;

    auto shader = assets->get<ShaderProgram>("ui3d");
    auto atlas = assets->get<Atlas>("blocks");

    Viewport viewport(iconSize, iconSize);
    DrawContext pctx(nullptr, viewport, nullptr);
    DrawContext ctx = pctx.sub();
    ctx.setCullFace(true);
    ctx.setDepthTest(true);

    Framebuffer fbo(iconSize, iconSize, true);
    Batch3D batch(1024);
    batch.begin();

    shader->use();
    shader->uniformMatrix(
        "u_projview",
        glm::ortho(0.0f, float(iconSize), 0.0f, float(iconSize), -100.0f, 100.0f) * glm::lookAt(glm::vec3(0.57735f), glm::vec3(0.0f), glm::vec3(0, 1, 0))
    );

    AtlasBuilder builder;
    Window::viewport(0, 0, iconSize, iconSize);
    Window::setBgColor(glm::vec4(0.0f));

    fbo.bind();
    for (size_t i = 0; i < count; ++i) {
        auto& def = indices->blocks.require(i);

        atlas->getTexture()->bind();

        builder.add(def.name, draw(cache, shader, &fbo, &batch, def, iconSize));
    }
    fbo.unbind();

    Window::viewport(0, 0, Window::width, Window::height);
    return builder.build(2);
}
