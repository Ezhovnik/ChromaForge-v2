#pragma once

class LineBatch;
class DrawContext;
class Camera;
class ShaderProgram;

class GuidesRenderer {
public:
    void drawBorders(
        LineBatch& batch, int sx, int sy, int sz, int ex, int ey, int ez
    );
    void drawCoordSystem(
        LineBatch& batch, const DrawContext& pctx, float length
    );

    void renderDebugLines(
        const DrawContext& context,
        const Camera& camera,
        LineBatch& batch,
        ShaderProgram& linesShader,
        bool showChunkBorders
    );
};
