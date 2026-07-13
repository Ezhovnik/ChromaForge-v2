#include <graphics/render/BlocksPreview.h>

#include <glm/ext.hpp>

#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/Atlas.h>
#include <graphics/core/Batch3D.h>
#include <window/Camera.h>
#include <voxels/Block.h>
#include <frontend/ContentGfxCache.h>
#include <assets/Assets.h>
#include <graphics/core/Framebuffer.h>
#include <graphics/core/DrawContext.h>
#include <window/Window.h>
#include <content/Content.h>
#include <constants.h>
#include <graphics/core/ImageData.h>
#include <graphics/commons/Model.h>

std::unique_ptr<ImageData> BlocksPreview::draw(
    const ContentGfxCache& cache,
    ShaderProgram& shader,
    const Framebuffer& fbo,
    Batch3D& batch,
    const Block& def, 
    int size
) {
    display::clear();
    blockid_t id = def.rt.id;
    const UVRegion texfaces[6]{
        cache.getRegion(id, 0, 0),
        cache.getRegion(id, 0, 1),
        cache.getRegion(id, 0, 2),
        cache.getRegion(id, 0, 3),
        cache.getRegion(id, 0, 4),
        cache.getRegion(id, 0, 5)
    };

    glm::vec3 offset(0.1f, 0.5f, 0.1f);
    switch (def.defaults.model.type) {
        case BlockModelType::None:
            break;
        case BlockModelType::Cube:
            shader.uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
            batch.blockCube(glm::vec3(size * 0.63f), texfaces, glm::vec4(1.0f), !def.rt.emissive);
            batch.flush();
            break;
        case BlockModelType::AABB:
            {
                glm::vec3 hitbox {};
                for (const auto& box : def.hitboxes) {
                    hitbox = glm::max(hitbox, box.size());
                }
                offset = glm::vec3(1, 1, 0.0f);
                shader.uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
                glm::vec3 scaledSize = glm::vec3(size * 0.63f);
                batch.cube(
                    -hitbox * scaledSize * 0.5f * glm::vec3(1, 1, -1),
                    hitbox * scaledSize, 
                    texfaces, glm::vec4(1.0f), 
                    !def.rt.emissive
                );
            }
            batch.flush();
            break;
        case BlockModelType::Custom:{
            glm::vec3 pmul = glm::vec3(size * 0.63f);
            glm::vec3 hitbox = glm::vec3(1.0f);
            glm::vec3 poff = glm::vec3(0.0f, 0.0f, 1.0f);
            offset.y += (1.0f - hitbox).y * 0.5f;
            shader.uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
            const auto& model = cache.getModel(def.rt.id);

            for (const auto& mesh : model.meshes) {
                for (const auto& vertex : mesh.vertices) {
                    float d = glm::dot(glm::normalize(vertex.normal), glm::vec3(0.2, 0.8, 0.4));
                    d = 0.8f + d * 0.2f;
                    batch.vertex((vertex.coord - poff)*pmul, vertex.uv, glm::vec4(d, d, d, 1.0f));
                }
                batch.flush();
            }
            break;
        } 
        case BlockModelType::X: {
            shader.uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
            glm::vec3 right = glm::normalize(glm::vec3(1.0f, 0.0f, -1.0f));
            batch.sprite(
                right * float(size) * 0.43f + glm::vec3(0, size * 0.4f, 0), 
                glm::vec3(0.0f, 1.0f, 0.0f), 
                right, 
                size * 0.5f, size * 0.6f, 
                texfaces[0], 
                glm::vec4(1.0f)
            );
            batch.flush();
            break;
        }
    }
    return fbo.getTexture()->readData();
}

std::unique_ptr<Atlas> BlocksPreview::build(
    Window& window,
    const ContentGfxCache& cache,
    const Assets& assets, 
    const ContentIndices& indices
) {
    size_t count = indices.blocks.count();
    size_t iconSize = ITEM_ICON_SIZE;

    auto& shader = assets.require<ShaderProgram>("ui3d");
    const auto& atlas = assets.require<Atlas>("blocks");

    DrawContext pctx(nullptr, window, nullptr);
    DrawContext ctx = pctx.sub();
    ctx.setCullFace(true);
    ctx.setDepthTest(true);

    Framebuffer fbo(iconSize, iconSize, true);
    Batch3D batch(1024);
    batch.begin();

    shader.use();
    shader.uniformMatrix(
        "u_projview",
        glm::ortho(0.0f, float(iconSize), 0.0f, float(iconSize), -100.0f, 100.0f) * glm::lookAt(glm::vec3(0.57735f), glm::vec3(0.0f), glm::vec3(0, 1, 0))
    );

    AtlasBuilder builder;
    ctx.setViewport({iconSize, iconSize});
    display::setBgColor(glm::vec4(0.0f));

    fbo.bind();
    for (size_t i = 0; i < count; ++i) {
        auto& def = indices.blocks.require(i);
        atlas.getTexture()->bind();
        builder.add(def.name, draw(cache, shader, fbo, batch, def, iconSize));
    }
    fbo.unbind();

    return builder.build(2);
}
