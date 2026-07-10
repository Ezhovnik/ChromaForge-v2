#include <graphics/render/GuidesRenderer.h>

#include <glm/gtc/matrix_transform.hpp>

#include <graphics/core/ShaderProgram.h>
#include <graphics/core/LineBatch.h>
#include <graphics/core/DrawContext.h>
#include <math/voxmaths.h>
#include <window/Camera.h>
#include <constants.h>

void GuidesRenderer::drawBorders(
    LineBatch& batch, int start_x, int start_y, int start_z, int end_x, int end_y, int end_z
) {
    int ww = end_x - start_x;
	int dd = end_z - start_z;
    {
		batch.line(start_x, start_y, start_z, start_x, end_y, start_z, 0.8f, 0, 0.8f, 1);
		batch.line(start_x, start_y, end_z, start_x, end_y, end_z, 0.8f, 0, 0.8f, 1);
		batch.line(end_x, start_y, start_z, end_x, end_y, start_z, 0.8f, 0, 0.8f, 1);
		batch.line(end_x, start_y, end_z, end_x, end_y, end_z, 0.8f, 0, 0.8f, 1);
	}
	for (int i = 2; i < ww; i += 2) {
		batch.line(start_x + i, start_y, start_z, start_x + i, end_y, start_z, 0, 0, 0.8f, 1);
		batch.line(start_x + i, start_y, end_z, start_x + i, end_y, end_z, 0, 0, 0.8f, 1);
	}
	for (int i = 2; i < dd; i += 2) {
		batch.line(start_x, start_y, start_z + i, start_x, end_y, start_z + i, 0.8f, 0, 0, 1);
		batch.line(end_x, start_y, start_z + i, end_x, end_y, start_z + i, 0.8f, 0, 0, 1);
	}
	for (int i = start_y; i < end_y; i += 2){
		batch.line(start_x, i, start_z, start_x, i, end_z, 0, 0.8f, 0, 1);
		batch.line(start_x, i, end_z, end_x, i, end_z, 0, 0.8f, 0, 1);
		batch.line(end_x, i, end_z, end_x, i, start_z, 0, 0.8f, 0, 1);
		batch.line(end_x, i, start_z, start_x, i, start_z, 0, 0.8f, 0, 1);
	}
	batch.flush();
}

void GuidesRenderer::drawCoordSystem(
    LineBatch& batch, const DrawContext& pctx, float length
) {
    auto ctx = pctx.sub();
    ctx.setDepthTest(false);
    batch.setLineWidth(4.0f);
    batch.line(0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
    batch.line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 1.f);
    batch.line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 1.f);
    batch.flush();

    ctx.setDepthTest(true);
    batch.setLineWidth(2.0f);
    batch.line(0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
    batch.line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 1.f);
    batch.line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 1.f);
}

void GuidesRenderer::renderDebugLines(
    const DrawContext& pctx,
    const Camera& camera,
    LineBatch& batch,
    ShaderProgram& linesShader,
    bool showChunkBorders
) {
    DrawContext ctx = pctx.sub(&batch);
    const auto& viewport = ctx.getViewport();

    ctx.setDepthTest(true);

    linesShader.use();

    if (showChunkBorders) {
        linesShader.uniformMatrix("u_projview", camera.getProjView());
        glm::vec3 coord = camera.position;
        if (coord.x < 0) coord.x--;
        if (coord.z < 0) coord.z--;
        int cx = floordiv(static_cast<int>(coord.x), CHUNK_WIDTH);
        int cz = floordiv(static_cast<int>(coord.z), CHUNK_DEPTH);

        drawBorders(
            batch,
            cx * CHUNK_WIDTH,
            0,
            cz * CHUNK_DEPTH,
            (cx + 1) * CHUNK_WIDTH,
            CHUNK_HEIGHT,
            (cz + 1) * CHUNK_DEPTH
        );
    }

    float length = 40.f;
    glm::vec3 tsl(viewport.x / 2, viewport.y / 2, 0.f);
    glm::mat4 model(glm::translate(glm::mat4(1.f), tsl));
    linesShader.uniformMatrix(
        "u_projview",
        glm::ortho(
            0.f,
            static_cast<float>(viewport.x),
            0.f,
            static_cast<float>(viewport.y),
            -length,
            length
        ) * model * glm::inverse(camera.rotation)
    );
    drawCoordSystem(batch, ctx, length);
}
