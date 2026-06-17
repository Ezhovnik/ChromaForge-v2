#pragma once

#include <stdexcept>
#include <set>
#include <algorithm>

#include <glm/glm.hpp>

#include <voxels/voxel.h>
#include <voxels/Block.h>
#include <voxels/Chunk.h>
#include <voxels/Chunks.h>
#include <voxels/GlobalChunks.h>
#include <content/Content.h>
#include <math/voxmaths.h>
#include <typedefs.h>
#include <constants.h>
#include <voxels/VoxelsVolume.h>

struct AABB;

namespace blocks_agent {
    template<class Storage>
    inline Chunk* get_chunk(const Storage& chunks, int cx, int cz) {
        return chunks.getChunk(cx, cz);
    }

    template<class Storage>
    inline voxel* get(const Storage& chunks, int32_t x, int32_t y, int32_t z) {
        if (y < 0 || y >= CHUNK_HEIGHT) {
            return nullptr;
        }
        int cx = floordiv<CHUNK_WIDTH>(x);
        int cz = floordiv<CHUNK_DEPTH>(z);
        Chunk* chunk = get_chunk(chunks, cx, cz);
        if (chunk == nullptr) {
            return nullptr;
        }
        int lx = x - cx * CHUNK_WIDTH;
        int lz = z - cz * CHUNK_DEPTH;
        return &chunk->voxels[(y * CHUNK_DEPTH + lz) * CHUNK_WIDTH + lx];
    }

    template<class Storage>
    inline voxel& require(const Storage& chunks, int32_t x, int32_t y, int32_t z) {
        auto vox = get(chunks, x, y, z);
        if (vox == nullptr) {
            throw std::runtime_error("Voxel does not exist");
        }
        return *vox;
    }

    template<class Storage>
    inline const Block& get_block_def(const Storage& chunks, blockid_t id) {
        return chunks.getContentIndices().blocks.require(id);
    }

    template<class Storage>
    inline bool is_solid_at(const Storage& chunks, int32_t x, int32_t y, int32_t z) {
        if (auto vox = get(chunks, x, y, z)) {
            return get_block_def(chunks, vox->id).rt.solid;
        }
        return false;
    }

    template<class Storage>
    inline bool is_replaceable_at(const Storage& chunks, int32_t x, int32_t y, int32_t z) {
        if (auto vox = get(chunks, x, y, z)) {
            return get_block_def(chunks, vox->id).replaceable;
        }
        return false;
    }

    void set(
        Chunks& chunks,
        int32_t x,
        int32_t y,
        int32_t z,
        uint32_t id,
        blockstate state
    );

    void set(
        GlobalChunks& chunks,
        int32_t x,
        int32_t y,
        int32_t z,
        uint32_t id,
        blockstate state
    );

    template<class Storage>
    inline void erase_segments(
        Storage& chunks, const Block& def, blockstate state, int x, int y, int z
    ) {
        const auto& rotation = def.rotations.variants[state.rotation];
        for (int sy = 0; sy < def.size.y; ++sy) {
            for (int sz = 0; sz < def.size.z; ++sz) {
                for (int sx = 0; sx < def.size.x; ++sx) {
                    if ((sx | sy | sz) == 0) continue;
                    glm::ivec3 pos(x, y, z);
                    pos += rotation.axes[0] * sx;
                    pos += rotation.axes[1] * sy;
                    pos += rotation.axes[2] * sz;
                    set(chunks, pos.x, pos.y, pos.z, 0, {});
                }
            }
        }
    }

    static constexpr uint8_t segment_to_int(int sx, int sy, int sz) {
        return ((sx > 0) | ((sy > 0) << 1) | ((sz > 0) << 2));
    }

    template <class Storage>
    inline void repair_segments(
        Storage& chunks, const Block& def, blockstate state, int x, int y, int z
    ) {
        const auto& rotation = def.rotations.variants[state.rotation];
        const auto id = def.rt.id;
        const auto size = def.size;
        for (int sy = 0; sy < size.y; ++sy) {
            for (int sz = 0; sz < size.z; ++sz) {
                for (int sx = 0; sx < size.x; ++sx) {
                    if ((sx | sy | sz) == 0) continue;
                    blockstate segState = state;
                    segState.segment = segment_to_int(sx, sy, sz);

                    glm::ivec3 pos(x, y, z);
                    pos += rotation.axes[0] * sx;
                    pos += rotation.axes[1] * sy;
                    pos += rotation.axes[2] * sz;
                    set(chunks, pos.x, pos.y, pos.z, id, segState);
                }
            }
        }
    }

    template <class Storage>
    inline glm::ivec3 seek_origin(
        Storage& chunks, const glm::ivec3& srcpos, const Block& def, blockstate state
    ) {
        auto pos = srcpos;
        const auto& rotation = def.rotations.variants[state.rotation];
        auto segment = state.segment;
        while (true) {
            if (!segment) return pos;
            if (segment & 1) pos -= rotation.axes[0];
            if (segment & 2) pos -= rotation.axes[1];
            if (segment & 4) pos -= rotation.axes[2];

            if (auto* voxel = get(chunks, pos.x, pos.y, pos.z)) {
                segment = voxel->state.segment;
            } else {
                return pos;
            }
        }
    }

    template <class Storage>
    inline bool check_replaceability(
        const Storage& chunks,
        const Block& def,
        blockstate state,
        const glm::ivec3& origin,
        blockid_t ignore
    ) {
        const auto& blocks = chunks.getContentIndices().blocks;
        const auto& rotation = def.rotations.variants[state.rotation];
        const auto size = def.size;
        for (int sy = 0; sy < size.y; ++sy) {
            for (int sz = 0; sz < size.z; ++sz) {
                for (int sx = 0; sx < size.x; ++sx) {
                    auto pos = origin;
                    pos += rotation.axes[0] * sx;
                    pos += rotation.axes[1] * sy;
                    pos += rotation.axes[2] * sz;
                    if (auto vox = get(chunks, pos.x, pos.y, pos.z)) {
                        auto& target = blocks.require(vox->id);
                        if (!target.replaceable && vox->id != ignore) {
                            return false;
                        }
                    } else {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    template <class Storage>
    inline void set_rotation_extended(
        Storage& chunks,
        const Block& def,
        blockstate state,
        const glm::ivec3& origin,
        uint8_t index
    ) {
        auto newstate = state;
        newstate.rotation = index;

        if (!check_replaceability(chunks, def, newstate, origin, def.rt.id)) {
            return;
        }

        const auto& rotation = def.rotations.variants[index];
        const auto size = def.size;
        std::vector<glm::ivec3> segmentBlocks;

        for (int sy = 0; sy < size.y; ++sy) {
            for (int sz = 0; sz < size.z; ++sz) {
                for (int sx = 0; sx < size.x; ++sx) {
                    auto pos = origin;
                    pos += rotation.axes[0] * sx;
                    pos += rotation.axes[1] * sy;
                    pos += rotation.axes[2] * sz;

                    blockstate segState = newstate;
                    segState.segment = segment_to_int(sx, sy, sz);

                    auto vox = get(chunks, pos.x, pos.y, pos.z);
                    if (vox->id != def.rt.id) {
                        set(chunks, pos.x, pos.y, pos.z, def.rt.id, segState);
                    } else {
                        vox->state = segState;
                        int cx = floordiv<CHUNK_WIDTH>(pos.x);
                        int cz = floordiv<CHUNK_DEPTH>(pos.z);
                        auto chunk = get_chunk(chunks, cx, cz);
                        assert(chunk != nullptr);
                        chunk->setModifiedAndUnsaved();
                        segmentBlocks.emplace_back(pos);
                    }
                }
            }
        }
        const auto& prevRotation = def.rotations.variants[state.rotation];
        for (int sy = 0; sy < size.y; ++sy) {
            for (int sz = 0; sz < size.z; ++sz) {
                for (int sx = 0; sx < size.x; ++sx) {
                    auto pos = origin;
                    pos += prevRotation.axes[0] * sx;
                    pos += prevRotation.axes[1] * sy;
                    pos += prevRotation.axes[2] * sz;
                    if (std::find(segmentBlocks.begin(), segmentBlocks.end(), pos) == segmentBlocks.end()) {
                        set(chunks, pos.x, pos.y, pos.z, 0, {});
                    }
                }
            }
        }
    }

    template <class Storage>
    inline void set_rotation(
        Storage& chunks, int32_t x, int32_t y, int32_t z, uint8_t index
    ) {
        if (index >= BlockRotProfile::MAX_COUNT) return;

        auto vox = get(chunks, x, y, z);
        if (vox == nullptr) return;

        const auto& def = chunks.getContentIndices().blocks.require(vox->id);
        if (!def.rotatable || vox->state.rotation == index) {
            return;
        }
        if (def.rt.extended) {
            auto origin = seek_origin(chunks, {x, y, z}, def, vox->state);
            vox = get(chunks, origin.x, origin.y, origin.z);
            set_rotation_extended(chunks, def, vox->state, origin, index);
        } else {
            vox->state.rotation = index;
            int cx = floordiv<CHUNK_WIDTH>(x);
            int cz = floordiv<CHUNK_DEPTH>(z);
            auto chunk = get_chunk(chunks, cx, cz);
            assert(chunk != nullptr);
            chunk->setModifiedAndUnsaved();
        }
    }

    voxel* raycast(
        const Chunks& chunks,
        const glm::vec3& start,
        const glm::vec3& dir,
        float maxDist,
        glm::vec3& end,
        glm::ivec3& norm,
        glm::ivec3& iend,
        std::set<blockid_t> filter
    );

    voxel* raycast(
        const GlobalChunks& chunks,
        const glm::vec3& start,
        const glm::vec3& dir,
        float maxDist,
        glm::vec3& end,
        glm::ivec3& norm,
        glm::ivec3& iend,
        std::set<blockid_t> filter
    );

    void get_voxels(const Chunks& chunks, VoxelsVolume* volume, bool backlight=false);

    void get_voxels(const GlobalChunks& chunks, VoxelsVolume* volume, bool backlight=false);

    template <class Storage>
    inline const AABB* is_obstacle_at(const Storage& chunks, float x, float y, float z) {
        int ix = std::floor(x);
        int iy = std::floor(y);
        int iz = std::floor(z);
        voxel* v = get(chunks, ix, iy, iz);
        if (v == nullptr) {
            if (iy >= CHUNK_HEIGHT) {
                return nullptr;
            } else {
                static const AABB empty;
                return &empty;
            }
        }
        const auto& def = chunks.getContentIndices().blocks.require(v->id);
        if (def.obstacle) {
            glm::ivec3 offset {};
            if (v->state.segment) {
                glm::ivec3 point(ix, iy, iz);
                offset = seek_origin(chunks, point, def, v->state) - point;
            }
            const auto& boxes = def.rotatable ? def.rt.hitboxes[v->state.rotation] : def.hitboxes;
            for (const auto& hitbox : boxes) {
                if (hitbox.contains(
                    {
                        x - ix - offset.x,
                        y - iy - offset.y,
                        z - iz - offset.z
                    }
                )) {
                    return &hitbox;
                }
            }
        }
        return nullptr;
    }
} // blocks_agent
