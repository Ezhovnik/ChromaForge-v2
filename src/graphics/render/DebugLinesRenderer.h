#pragma once

class DrawContext;
class Camera;
class LineBatch;
class LinesRenderer;
class ShaderProgram;
class Level;

class DebugLinesRenderer {
public:
    static bool drawPaths;

    DebugLinesRenderer(const Level& level) : level(level) {};

    void render(
        DrawContext& ctx,
        const Camera& camera,
        LinesRenderer& renderer,
        LineBatch& linesBatch,
        ShaderProgram& linesShader,
        bool showChunkBorders
    );
private:
    const Level& level;

    void drawBorders(
        LineBatch& batch, int sx, int sy, int sz, int ex, int ey, int ez
    );
    void drawCoordSystem(
        LineBatch& batch, const DrawContext& pctx, float length
    );

};
