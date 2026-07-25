#include <graphics/render/DebugLinesRenderer.h>

#include <graphics/core/ShaderProgram.h>
#include <window/Camera.h>
#include <graphics/core/LineBatch.h>
#include <graphics/core/DrawContext.h>
#include <graphics/render/LinesRenderer.h>
#include <world/Level.h>
#include <voxels/Chunk.h>
#include <voxels/Pathfinding.h>
#include <math/voxmaths.h>

bool DebugLinesRenderer::drawPaths = false;

static void draw_route(
    LinesRenderer& lines, const voxels::Agent& agent
) {
    const auto& route = agent.route;
    if (!route.found) return;

    for (int i = 1; i < route.nodes.size(); ++i) {
        const auto& a = route.nodes.at(i - 1);
        const auto& b = route.nodes.at(i);

        if (i == 1) {
            lines.pushLine(
                glm::vec3(a.pos) + glm::vec3(0.5f),
                glm::vec3(a.pos) + glm::vec3(0.5f, 1.0f, 0.5f),
                glm::vec4(1, 1, 1, 1)
            );
        }

        lines.pushLine(
            glm::vec3(a.pos) + glm::vec3(0.5f),
            glm::vec3(b.pos) + glm::vec3(0.5f),
            glm::vec4(1, 0, 1, 1)
        );

        lines.pushLine(
            glm::vec3(b.pos) + glm::vec3(0.5f),
            glm::vec3(b.pos) + glm::vec3(0.5f, 1.0f, 0.5f),
            glm::vec4(1, 1, 1, 1)
        );
    }
}

void DebugLinesRenderer::drawBorders(
    LineBatch& batch, int sx, int sy, int sz, int ex, int ey, int ez
) {
    int ww = ex - sx;
    int dd = ez - sz;
    {
        batch.line(sx, sy, sz, sx, ey, sz, 0.8f, 0, 0.8f, 1);
        batch.line(sx, sy, ez, sx, ey, ez, 0.8f, 0, 0.8f, 1);
        batch.line(ex, sy, sz, ex, ey, sz, 0.8f, 0, 0.8f, 1);
        batch.line(ex, sy, ez, ex, ey, ez, 0.8f, 0, 0.8f, 1);
    }
    for (int i = 2; i < ww; i += 2) {
        batch.line(sx + i, sy, sz, sx + i, ey, sz, 0, 0, 0.8f, 1);
        batch.line(sx + i, sy, ez, sx + i, ey, ez, 0, 0, 0.8f, 1);
    }
    for (int i = 2; i < dd; i += 2) {
        batch.line(sx, sy, sz + i, sx, ey, sz + i, 0.8f, 0, 0, 1);
        batch.line(ex, sy, sz + i, ex, ey, sz + i, 0.8f, 0, 0, 1);
    }
    for (int i = sy; i < ey; i += 2) {
        batch.line(sx, i, sz, sx, i, ez, 0, 0.8f, 0, 1);
        batch.line(sx, i, ez, ex, i, ez, 0, 0.8f, 0, 1);
        batch.line(ex, i, ez, ex, i, sz, 0, 0.8f, 0, 1);
        batch.line(ex, i, sz, sx, i, sz, 0, 0.8f, 0, 1);
    }
    batch.flush();
}

void DebugLinesRenderer::drawCoordSystem(
    LineBatch& batch, const DrawContext& pctx, float length
) {
    auto ctx = pctx.sub();
    ctx.setDepthTest(false);
    batch.setLineWidth(4.0f);
    batch.line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    batch.line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    batch.line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 1.0f);
    batch.flush();

    ctx.setDepthTest(true);
    batch.setLineWidth(2.0f);
    batch.line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    batch.line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    batch.line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 1.0f);
}


void DebugLinesRenderer::render(
    DrawContext& pctx,
    const Camera& camera,
    LinesRenderer& renderer,
    LineBatch& linesBatch,
    ShaderProgram& linesShader,
    bool showChunkBorders
) {
    if (drawPaths) {
        for (const auto& [_, agent] : level.pathfinding->getAgents()) {
            draw_route(renderer, agent);
        }
    }
    DrawContext ctx = pctx.sub(&linesBatch);
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
            linesBatch,
            cx * CHUNK_WIDTH,
            0,
            cz * CHUNK_DEPTH,
            (cx + 1) * CHUNK_WIDTH,
            CHUNK_HEIGHT,
            (cz + 1) * CHUNK_DEPTH
        );
    }

    float length = 40.0f;
    glm::vec3 tsl(viewport.x / 2, viewport.y / 2, 0.0f);
    glm::mat4 model(glm::translate(glm::mat4(1.0f), tsl));
    linesShader.uniformMatrix(
        "u_projview",
        glm::ortho(
            0.0f,
            static_cast<float>(viewport.x),
            0.0f,
            static_cast<float>(viewport.y),
            -length,
            length
        ) * model * glm::inverse(camera.rotation)
    );
    drawCoordSystem(linesBatch, ctx, length);
}
