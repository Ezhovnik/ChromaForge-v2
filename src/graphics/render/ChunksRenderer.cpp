#include <graphics/render/ChunksRenderer.h>

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
    const Chunks& chunks;
    BlocksRenderer renderer;
public:
    RendererWorker(
        const Level& level,
        const Chunks& chunks,
        const ContentGfxCache& cache,
        const EngineSettings& settings
    ) : chunks(chunks),
        renderer(
            settings.graphics.denseRender.get()
                ? settings.graphics.chunkMaxVerticesDense.get()
                : settings.graphics.chunkMaxVertices.get(),
            level.content,
            cache,
            settings
        ) {}

    RendererResult operator()(const std::shared_ptr<Chunk>& chunk) override {
        renderer.build(chunk.get(), &chunks);
        if (renderer.isCancelled()) {
            return RendererResult {
                glm::ivec2(chunk->chunk_x, chunk->chunk_z), true, ChunkMeshData {}
            };
        }
        auto meshData = renderer.createMesh();
        return RendererResult {
            glm::ivec2(chunk->chunk_x, chunk->chunk_z), false, std::move(meshData)
        };
    }
};

ChunksRenderer::ChunksRenderer(
    const Level* level,
    const Chunks& chunks,
    const Assets& assets,
    const Frustum& frustum,
    const ContentGfxCache& cache,
    const EngineSettings& settings
) : chunks(chunks),
    assets(assets),
    frustum(frustum),
    settings(settings),
    threadPool(
        "chunks-render-pool",
        [&](){
            return std::make_shared<RendererWorker>(
                *level, chunks, cache, settings
            );
        }, 
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
        settings.graphics.chunkMaxVertices.get(), level->content, cache, settings
    );

    LOG_INFO("Created {} workers", threadPool.getWorkersCount());
}

ChunksRenderer::~ChunksRenderer() {
}

const Mesh* ChunksRenderer::render(
    const std::shared_ptr<Chunk>& chunk,
    bool important
) {
    chunk->flags.modified = false;

    if (important) {
        auto mesh = renderer->render(chunk.get(), &chunks);
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

    if (chunk->flags.modified && chunk->flags.lighted) render(chunk, important);

    return found->second.mesh.get();
}

void ChunksRenderer::update() {
    threadPool.update();
}

const Mesh* ChunksRenderer::retrieveChunk(
    size_t index, const Camera& camera, ShaderProgram& shader, bool culling
) {
    auto chunk = chunks.getChunks()[index];
    if (chunk == nullptr) return nullptr;
    if (!chunk->flags.lighted) {
        const auto& found = meshes.find({chunk->chunk_x, chunk->chunk_z});
        if (found == meshes.end()) {
            return nullptr;
        } else {
            return found->second.mesh.get();
        }
    }

    float distance = glm::distance(
        camera.position,
        glm::vec3(
            (chunk->chunk_x + 0.5f) * CHUNK_WIDTH,
            camera.position.y,
            (chunk->chunk_z + 0.5f) * CHUNK_DEPTH
        )
    );
    auto mesh = getOrRender(chunk, distance < CHUNK_WIDTH * 1.5f);
    if (mesh == nullptr) return nullptr;

    if (culling) {
        glm::vec3 min(chunk->chunk_x * CHUNK_WIDTH, chunk->bottom, chunk->chunk_z * CHUNK_DEPTH);
        glm::vec3 max(
            chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH,
            chunk->top,
            chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH
        );

        if (!frustum.isBoxVisible(min, max)) return nullptr;
    }
    return mesh;
}

void ChunksRenderer::drawChunks(
    const Camera& camera, ShaderProgram& shader
) {
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

    // TODO: minimize the number of draw calls
    for (int i = indices.size() - 1; i >= 0; --i) {
        auto& chunk = chunks.getChunks()[indices[i].index];
        auto mesh = retrieveChunk(indices[i].index, camera, shader, culling);

        if (mesh) {
            glm::vec3 coord(
                chunk->chunk_x * CHUNK_WIDTH + 0.5f,
                0.5f,
                chunk->chunk_z * CHUNK_DEPTH + 0.5f
            );
            glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);
            shader.uniformMatrix("u_model", model);
            mesh->draw();
            visibleChunks++;
        }
    }
}

static inline void write_sorting_mesh_entries(
    float* buffer, const std::vector<SortingMeshEntry>& chunkEntries
) {
    for (const auto& entry : chunkEntries) {
        const auto& vertexData = entry.vertexData;
        std::memcpy(
            buffer,
            vertexData.data(),
            vertexData.size() * sizeof(float)
        );
        buffer += vertexData.size();
    }
}

void ChunksRenderer::drawSortedMeshes(const Camera& camera, ShaderProgram& shader) {
    const int sortInterval = TRANSLUCENT_BLOCKS_SORT_INTERVAL;
    static int frameid = 0;
    frameid++;

    bool culling = settings.graphics.frustumCulling.get();
    const auto& chunks = this->chunks.getChunks();
    const auto& cameraPos = camera.position;

    const auto& atlas = assets.require<Atlas>("blocks");
    shader.use();
    atlas.getTexture()->bind();

    shader.uniformMatrix("u_model", glm::mat4(1.0f));
    shader.uniform1i("u_alphaClip", false);

    for (const auto& index : indices) {
        const auto& chunk = chunks[index.index];
        if (chunk == nullptr || !chunk->flags.lighted) continue;

        const auto& found = meshes.find(glm::ivec2(chunk->chunk_x, chunk->chunk_z));
        if (found == meshes.end() || found->second.sortingMeshData.entries.empty()) continue;

        if (culling) {
            glm::vec3 min(
                chunk->chunk_x * CHUNK_WIDTH,
                chunk->bottom,
                chunk->chunk_z * CHUNK_DEPTH
            );
            glm::vec3 max(
                chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH,
                chunk->top,
                chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH
            );

            if (!frustum.isBoxVisible(min, max)) continue;
        }

        auto& chunkEntries = found->second.sortingMeshData.entries;

        if (chunkEntries.size() == 1) {
            auto& entry = chunkEntries.at(0);
            if (found->second.sortedMesh == nullptr) {
                found->second.sortedMesh = std::make_unique<Mesh>(
                    entry.vertexData.data(),
                    entry.vertexData.size() / CHUNK_VERTEX_SIZE,
                    CHUNK_VATTRS
                );
            }
            found->second.sortedMesh->draw();
            continue;
        }

        for (auto& entry : chunkEntries) {
            entry.distance = static_cast<long long>(
                glm::distance2(entry.position, cameraPos)
            );
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
            write_sorting_mesh_entries(buffer.data(), chunkEntries);
            found->second.sortedMesh = std::make_unique<Mesh>(
                buffer.data(), size / CHUNK_VERTEX_SIZE, CHUNK_VATTRS
            );
        }
        found->second.sortedMesh->draw();
    }
}
