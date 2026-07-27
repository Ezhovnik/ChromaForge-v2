#include <gtest/gtest.h>
#include <cstdlib>

#include <voxels/Chunk.h>

TEST(Chunk, EncodeDecode) {
    using namespace voxels;
    Chunk chunk1(0, 0);
    for (uint i = 0; i < CHUNK_VOLUME; i++) {
        chunk1.voxels[i].id = rand();
        chunk1.voxels[i].state.rotation = rand();
        chunk1.voxels[i].state.segment = rand();
        chunk1.voxels[i].state.userbits = rand();
    }
    auto bytes = chunk1.encode();

    Chunk chunk2(0, 0);
    chunk2.decode(bytes.get());

    for (uint i = 0; i < CHUNK_VOLUME; i++) {
        EXPECT_EQ(chunk1.voxels[i].id, chunk2.voxels[i].id);
        EXPECT_EQ(
            blockstate2int(chunk1.voxels[i].state),
            blockstate2int(chunk2.voxels[i].state)
        );
    }
}
