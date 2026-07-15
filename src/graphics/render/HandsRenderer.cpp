#include <graphics/render/HandsRenderer.h>

#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <graphics/render/ModelBatch.h>
#include <content/Content.h>
#include <graphics/commons/Model.h>
#include <objects/rigging.h>
#include <window/Camera.h>

HandsRenderer::HandsRenderer(
    const Assets& assets,
    ModelBatch& modelBatch,
    std::shared_ptr<rigging::Skeleton> skeleton
) : assets(assets),
    modelBatch(modelBatch),
    skeleton(std::move(skeleton)) {}

void HandsRenderer::renderHands(
    const Camera& camera, float delta
) {
    auto& skeleton = *this->skeleton;
    const auto& config = *skeleton.config;

    modelBatch.setLightsOffset(camera.position);
    config.update(skeleton, glm::mat4(1.0f), glm::vec3());
    config.render(assets, modelBatch, skeleton, glm::mat4(1.0f), glm::vec3());
}
