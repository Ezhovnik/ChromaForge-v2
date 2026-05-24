#include <graphics/render/ChunksRenderer.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <graphics/core/Mesh.h>
#include <graphics/render/BlocksRenderer.h>
#include <voxels/Chunk.h>
#include <world/Level.h>
#include <debug/Logger.h>
#include <settings.h>

class RendererWorker : public util::Worker<std::shared_ptr<Chunk>, RendererResult> {
    Level* level;
    BlocksRenderer renderer;
public:
    RendererWorker(
        Level* level, 
        const ContentGfxCache* cache, 
        const EngineSettings* settings
    ) : level(level), 
        renderer(settings->graphics.chunkMaxVertices.get(), level->content, cache, settings) {}

    RendererResult operator()(const std::shared_ptr<Chunk>& chunk) override {
        renderer.build(chunk.get(), level->chunks.get());
        if (renderer.isCancelled()) {
            return RendererResult {
                glm::ivec2(chunk->chunk_x, chunk->chunk_z), true, MeshData()
            };
        }
        auto meshData = renderer.createMesh();
        return RendererResult {
            glm::ivec2(chunk->chunk_x, chunk->chunk_z), false, std::move(meshData)
        };
    }
};

ChunksRenderer::ChunksRenderer(
    Level* level, 
    const ContentGfxCache* cache, 
    const EngineSettings* settings
) : level(level),
    threadPool(
        "chunks-render-pool",
        [=](){return std::make_shared<RendererWorker>(level, cache, settings);}, 
        [=](RendererResult& result){
            if (!result.cancelled) {
                meshes[result.key] = std::make_shared<Mesh>(result.meshData);
            }
            inwork.erase(result.key);
        },
        settings->graphics.chunkMaxRenderers.get()
    )
{
    threadPool.setStopOnFail(false);
    renderer = std::make_unique<BlocksRenderer>(
        settings->graphics.chunkMaxVertices.get(), level->content, cache, settings
    );

    LOG_INFO("Created {} workers", threadPool.getWorkersCount());
}

ChunksRenderer::~ChunksRenderer() {
}

std::shared_ptr<Mesh> ChunksRenderer::render(
    const std::shared_ptr<Chunk>& chunk,
    bool important
) {
    chunk->flags.modified = false;

    if (important) {
        auto mesh = renderer->render(chunk.get(), level->chunks.get());
        meshes[glm::ivec2(chunk->chunk_x, chunk->chunk_z)] = mesh;
        return mesh;
    }

    glm::ivec2 key(chunk->chunk_x, chunk->chunk_z);
    if (inwork.find(key) != inwork.end()) return nullptr;

    inwork[key] = true;
    threadPool.enqueueJob(chunk);
    return nullptr;
}

void ChunksRenderer::unload(const Chunk* chunk) {
    auto found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
    if (found != meshes.end()) meshes.erase(found);
}

void ChunksRenderer::clear() {
    meshes.clear();
    inwork.clear();
    threadPool.clearQueue();
}

std::shared_ptr<Mesh> ChunksRenderer::getOrRender(
    const std::shared_ptr<Chunk>& chunk,
    bool important
) {
    auto found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
    if (found == meshes.end()) return render(chunk, important);

    if (chunk->flags.modified) render(chunk, important);

    return found->second;
}

std::shared_ptr<Mesh> ChunksRenderer::get(Chunk* chunk) {
    auto found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
    if (found != meshes.end()) return found->second;
    return nullptr;
}

void ChunksRenderer::update() {
    threadPool.update();
}
