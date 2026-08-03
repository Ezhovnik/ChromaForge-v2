#include <voxels/VoxelsVolume.h>

#include <assert.h>

#include <constants.h>
#include <content/Content.h>
#include <voxels/Block.h>

void VoxelsVolume::compressInto(
    VoxelsVolume& dst, const Content& content
) const {
    assert(
        dst.width < width && dst.height < height && dst.depth < depth
    );
    assert(
        width % dst.width == 0 &&
        height % dst.height == 0 &&
        depth % dst.depth == 0
    );

    int stepW = width / dst.width;
    int stepH = height / dst.height;
    int stepD = depth / dst.depth;
    int height = dst.height;
    int depth = dst.depth;
    int width = dst.width;

    auto dstVoxels = dst.getVoxels();
    auto dstLights = dst.getLights();

    const auto& blockDefs = content.getIndices()->blocks;

    for (int y = 0; y < height; ++y) {
        for (int z = 0; z < depth; ++z) {
            for (int x = 0; x < width; ++x) {
                voxel selectedVoxel {BLOCK_AIR, {}};
                light_t light = 0;
                for (int ly = 0; ly < stepH; ++ly) {
                    for (int lz = 0; lz < stepD; ++lz) {
                        for (int lx = 0; lx < stepW; ++lx) {
                            size_t srcIndex = vox_index(
                                x * stepW + lx,
                                y * stepH + ly,
                                z * stepD,
                                width, depth
                            );
                            auto vox = voxels[srcIndex];
                            if (vox.id == BLOCK_VOID) {
                                continue;
                            }
                            const auto& def = blockDefs.require(vox.id);
                            if (def.rt.solid) {
                                selectedVoxel = std::move(vox);
                            } else if (light == 0) {
                                light = lights[srcIndex];
                            }
                        }
                    }
                }
                size_t dstIndex = vox_index(x, y, z, dst.width, dst.depth);
                dstLights[dstIndex] = light;
                dstVoxels[dstIndex] = std::move(selectedVoxel);
            }
        }
    }
}
