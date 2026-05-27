#include <graphics/render/ChunksRenderer.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <graphics/core/Mesh.h>
#include <graphics/render/BlocksRenderer.h>
#include <voxels/Chunk.h>
#include <world/Level.h>
#include <debug/Logger.h>
#include <settings.h>
#include <assets/Assets.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/Atlas.h>
#include <voxels/Chunks.h>
#include <window/Camera.h>
#include <math/FrustumCulling.h>
#include <util/listutil.h>

size_t ChunksRenderer::visibleChunks = 0;

class RendererWorker : public util::Worker<std::shared_ptr<Chunk>, RendererResult> {
    const Level& level;
    BlocksRenderer renderer;
public:
    RendererWorker(
        const Level& level,
        const ContentGfxCache& cache,
        const EngineSettings& settings
    ) : level(level), 
        renderer(settings.graphics.chunkMaxVertices.get(), *level.content, cache, settings) {}

    RendererResult operator()(const std::shared_ptr<Chunk>& chunk) override {
        renderer.build(chunk.get(), level.chunks.get());
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

const vattr CR_ATTRS[]{{3}, {2}, {1}, {0}};
inline constexpr int CR_VERTEX_SIZE = 6;

ChunksRenderer::ChunksRenderer(
    const Level* level, 
    const Assets& assets,
    const Frustum& frustum,
    const ContentGfxCache& cache,
    const EngineSettings& settings
) : level(*level),
    assets(assets),
    frustum(frustum),
    settings(settings),
    threadPool(
        "chunks-render-pool",
        [&](){return std::make_shared<RendererWorker>(*level, cache, settings);}, 
        [&](RendererResult& result){
            if (!result.cancelled) {
                auto meshData = std::move(result.meshData);
                meshes[result.key] = ChunkMesh {
                    std::make_unique<Mesh>(meshData.mesh),
                    std::move(meshData.sortingMesh)
                };
            }
            inwork.erase(result.key);
        },
        settings.graphics.chunkMaxRenderers.get()
    )
{
    threadPool.setStopOnFail(false);
    renderer = std::make_unique<BlocksRenderer>(
        settings.graphics.chunkMaxVertices.get(), *level->content, cache, settings
    );

    LOG_INFO("Created {} workers", threadPool.getWorkersCount());

    float buf[1]{};
    sortedMesh = std::make_unique<Mesh>(buf, 0, CR_ATTRS);
}

ChunksRenderer::~ChunksRenderer() {
}

const Mesh* ChunksRenderer::render(
    const std::shared_ptr<Chunk>& chunk,
    bool important
) {
    chunk->flags.modified = false;

    if (important) {
        auto mesh = renderer->render(chunk.get(), level.chunks.get());
        meshes[glm::ivec2(chunk->chunk_x, chunk->chunk_z)] = ChunkMesh {
            std::move(mesh.mesh), std::move(mesh.sortingMeshData)
        };
        return meshes[glm::ivec2(chunk->chunk_x, chunk->chunk_z)].mesh.get();
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

const Mesh* ChunksRenderer::getOrRender(
    const std::shared_ptr<Chunk>& chunk,
    bool important
) {
    auto found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
    if (found == meshes.end()) return render(chunk, important);

    if (chunk->flags.modified) render(chunk, important);

    return found->second.mesh.get();
}

void ChunksRenderer::update() {
    threadPool.update();
}

bool ChunksRenderer::drawChunk(
    size_t index, const Camera& camera, ShaderProgram& shader, bool culling
) {
    auto chunk = level.chunks->getChunks()[index];
    if (chunk == nullptr || !chunk->flags.lighted) return false;

    float distance = glm::distance(
        camera.position,
        glm::vec3(
            (chunk->chunk_x + 0.5f) * CHUNK_WIDTH,
            camera.position.y,
            (chunk->chunk_z + 0.5f) * CHUNK_DEPTH
        )
    );
    auto mesh = getOrRender(chunk, distance < CHUNK_WIDTH * 1.5f);
    if (mesh == nullptr) return false;

    if (culling) {
        glm::vec3 min(chunk->chunk_x * CHUNK_WIDTH, chunk->bottom, chunk->chunk_z * CHUNK_DEPTH);
        glm::vec3 max(
            chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH,
            chunk->top,
            chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH
        );

        if (!frustum.isBoxVisible(min, max)) return false;
    }
    glm::vec3 coord(chunk->chunk_x * CHUNK_WIDTH + 0.5f, 0.5f, chunk->chunk_z * CHUNK_DEPTH + 0.5f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);
    shader.uniformMatrix("u_model", model);
    mesh->draw();
    return true;
}

void ChunksRenderer::drawChunks(
    const Camera& camera, ShaderProgram& shader
) {
    const auto& chunks = *level.chunks;
    const auto& atlas = assets.require<Atlas>("blocks");

    atlas.getTexture()->bind();
    update();

    int chunksWidth = chunks.getWidth();
    int chunksOffsetX = chunks.getOffsetX();
    int chunksOffsetZ = chunks.getOffsetZ();

    if (indices.size() != chunks.getVolume()) {
        indices.clear();
        for (int i = 0; i < chunks.getVolume(); i++) {
            indices.push_back(ChunksSortEntry {i, 0});
        }
    }
    float px = camera.position.x / static_cast<float>(CHUNK_WIDTH) - 0.5f;
    float pz = camera.position.z / static_cast<float>(CHUNK_DEPTH) - 0.5f;
    for (auto& index : indices) {
        float x = index.index % chunksWidth + chunksOffsetX - px;
        float z = index.index / chunksWidth + chunksOffsetZ - pz;
        index.d = (x * x + z * z) * 1024;
    }
    util::insertion_sort(indices.begin(), indices.end());

    bool culling = settings.graphics.frustumCulling.get();

    visibleChunks = 0;
    shader.uniform1i("u_alphaClip", true);
    // if (GLEW_ARB_multi_draw_indirect && false) {
        // TODO: implement Multi Draw Indirect chunks draw
    // } else {
        for (size_t i = 0; i < indices.size(); ++i) {
            visibleChunks += drawChunk(indices[i].index, camera, shader, culling);
        }
    // }
}

void ChunksRenderer::drawSortedMeshes(const Camera& camera, ShaderProgram& shader) {
    const int sortInterval = 6;
    static int frameid = 0;
    frameid++;

    bool culling = settings.graphics.frustumCulling.get();
    const auto& chunks = level.chunks->getChunks();
    const auto& cameraPos = camera.position;

    const auto& atlas = assets.require<Atlas>("blocks");
    atlas.getTexture()->bind();

    shader.uniformMatrix("u_model", glm::mat4(1.0f));
    shader.uniform1i("u_alphaClip", false);

    for (const auto& index : indices) {
        const auto& chunk = chunks[index.index];
        if (chunk == nullptr || !chunk->flags.lighted) continue;

        const auto& found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
        if (found == meshes.end()) continue;

        glm::vec3 min(
            chunk->chunk_x * CHUNK_WIDTH - CHUNK_WIDTH,
            0,
            chunk->chunk_z * CHUNK_DEPTH - CHUNK_DEPTH
        );
        glm::vec3 max(
            chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH * 2,
            CHUNK_HEIGHT,
            chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH * 2
        );

        if (!frustum.isBoxVisible(min, max)) continue;

        auto& chunkEntries = found->second.sortingMeshData.entries;

        if (chunkEntries.empty()) {
            continue;
        } else if (chunkEntries.size() == 1) {
            auto& entry = chunkEntries.at(0);
            if (found->second.sortedMesh == nullptr) {
                found->second.sortedMesh = std::make_unique<Mesh>(
                    entry.vertexData.data(),
                    entry.vertexData.size() / CR_VERTEX_SIZE,
                    CR_ATTRS
                );
            }
            found->second.sortedMesh->draw();
            continue;
        }

        for (auto& entry : chunkEntries) {
            entry.distance = static_cast<long long>(glm::distance2(entry.position, cameraPos));
        }

        if (found->second.sortedMesh == nullptr || (frameid + chunk->chunk_x) % sortInterval == 0) {
            std::sort(chunkEntries.begin(), chunkEntries.end());
            size_t size = 0;
            for (const auto& entry : chunkEntries) {
                size += entry.vertexData.size();
            }
            static util::Buffer<float> buffer;
            if (buffer.size() < size) {
                buffer = util::Buffer<float>(size);
            }
            size_t offset = 0;
            for (const auto& entry : chunkEntries) {
                std::memcpy(
                    (buffer.data() + offset),
                    entry.vertexData.data(),
                    entry.vertexData.size() * sizeof(float)
                );
                offset += entry.vertexData.size();
            }
            found->second.sortedMesh = std::make_unique<Mesh>(
                buffer.data(), size / CR_VERTEX_SIZE, CR_ATTRS
            );
        }
        found->second.sortedMesh->draw();
    }
}
