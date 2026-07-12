#include <graphics/ui/elements/ModelViewer.h>

#include <glm/ext.hpp>
#include <GL/glew.h> // TODO: remove

#include <assets/Assets.h>
#include <assets/assets_util.h>
#include <graphics/commons/Model.h>
#include <graphics/core/Batch2D.h>
#include <graphics/core/Batch3D.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Framebuffer.h>
#include <graphics/core/DrawContext.h>
#include <window/Window.h>
#include <graphics/ui/GUI.h>
#include <core_content_defs.h>

using namespace gui;

ModelViewer::ModelViewer(
    GUI& gui, const glm::vec2& size, const std::string& modelName
) : Container(gui, size),
    modelName(modelName),
    camera(),
    batch(std::make_unique<Batch3D>(1024)),
    fbo(std::make_unique<Framebuffer>(size.x, size.y))
{
    camera.perspective = true;
    camera.position = glm::vec3(2, 2, 2);
}

ModelViewer::~ModelViewer() = default;

void ModelViewer::setModel(const std::string& modelName) {
    this->modelName = modelName;
}

const std::string& ModelViewer::getModel() const {
    return modelName;
}

Camera& ModelViewer::getCamera() {
    return camera;
}

const Camera& ModelViewer::getCamera() const {
    return camera;
}

void ModelViewer::activate(float delta) {
    Container::activate(delta);

    auto& input = gui.getInput();

    if (!grabbing && hover && input.justClicked(Mousecode::BUTTON_3)) {
        grabbing = true;
    }
    if (grabbing && input.isClicked(Mousecode::BUTTON_3)) {
        auto cursor = input.getCursor();
        if (input.isPressed(Keycode::LEFT_SHIFT)) {
            center -= camera.right * (cursor.delta.x / size.x) * distance;
            center += camera.up * (cursor.delta.y / size.y) * distance;
        } else {
            rotation.x -= cursor.delta.x / size.x * glm::two_pi<float>();
            rotation.y -= cursor.delta.y / size.y * glm::two_pi<float>();
            rotation.y = glm::max(
                -glm::half_pi<float>(), glm::min(glm::half_pi<float>(), rotation.y)
            );
        }
    } else if (grabbing) {
        grabbing = false;
    }
    if (hover) {
        distance *= 1.0f - 0.2f * input.getScroll();
    }
    camera.rotation = glm::mat4(1.0f);
    camera.rotate(rotation.y, rotation.x, rotation.z);
    camera.position = center - camera.front * distance;
}

void ModelViewer::draw(const DrawContext& pctx, const Assets& assets) {
    camera.setAspectRatio(size.x / size.y);
    camera.updateVectors();

    auto model = assets.get<model::Model>(modelName);
    if (model == nullptr) return;

    auto& prevShader = ShaderProgram::getUsed();

    fbo->resize(size.x, size.y);
    {
        glDisable(GL_SCISSOR_TEST);
        auto ctx = pctx.sub();
        ctx.setFramebuffer(fbo.get());
        ctx.setViewport({size.x, size.y});
        ctx.setDepthTest(true);
        ctx.setCullFace(true);
        display::clear();

        auto& ui3dShader = assets.require<ShaderProgram>("ui3d");
        ui3dShader.use();
        ui3dShader.uniformMatrix("u_apply", glm::mat4(1.0f));
        ui3dShader.uniformMatrix("u_projview", camera.getProjView());
        batch->begin();
        for (const auto& mesh : model->meshes) {
            util::TextureRegion region;
            if (!mesh.texture.empty() && mesh.texture[0] == '$') {
                // TODO: refactor
                static std::array<std::string, 6> faces {
                    "blocks:debug_north",
                    "blocks:debug_south",
                    "blocks:debug_top",
                    "blocks:debug_bottom",
                    "blocks:debug_west",
                    "blocks:debug_east",
                };
                region = util::get_texture_region(
                    assets,
                    faces.at(mesh.texture.at(1) - '0'),
                    "blocks:" + TEXTURE_NOTFOUND
                );
            } else {
                region = util::get_texture_region(assets, mesh.texture, "blocks:notfound");
            }
            batch->texture(region.texture);
            batch->setRegion(region.region);
            for (const auto& vertex : mesh.vertices) {
                batch->vertex(vertex.coord, vertex.uv, vertex.normal);
            }
        }
        batch->flush();
        glEnable(GL_SCISSOR_TEST);
    }

    auto pos = calcPos();
    prevShader.use();
    auto& batch2d = *pctx.getBatch2D();
    batch2d.texture(fbo->getTexture());
    batch2d.rect(pos.x, pos.y, size.x, size.y, 0.0f, 0.0f, 0.0f, UVRegion {}, false, true, glm::vec4{1.0f});

    Container::draw(pctx, assets);
}

void ModelViewer::setCenter(const glm::vec3& center) {
    this->center = center;
}

void ModelViewer::setRotation(const glm::vec3& euler) {
    this->rotation = euler;
}
