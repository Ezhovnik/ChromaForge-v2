#include <voxels/compressed_chunks.h>

#include <coders/rle.h>
#include <coders/zip.h>
#include <coders/byte_utils.h>
#include <voxels/Chunk.h>

inline constexpr int HAS_VOXELS = 0x1;
inline constexpr int HAS_METADATA = 0x2;

std::vector<ubyte> compressed_chunks::encode(const Chunk& chunk) {
    auto data = chunk.encode();

    static util::Buffer<ubyte> rleBuffer;
    if (rleBuffer.size() < CHUNK_DATA_LEN * 2) {
        rleBuffer = util::Buffer<ubyte>(CHUNK_DATA_LEN * 2);
    }
    size_t rleCompressedSize = extrle::encode16(data.get(), CHUNK_DATA_LEN, rleBuffer.data());

    const auto zipCompressedData = zip::compress(
        rleBuffer.data(), rleCompressedSize
    );
    auto metadataBytes = chunk.blocksMetadata.serialize();

    ByteBuilder builder(2 + 8 + zipCompressedData.size() + metadataBytes.size());
    builder.put(HAS_VOXELS | HAS_METADATA); // Flags
    builder.put(0); // Reserved
    builder.putInt32(zipCompressedData.size());
    builder.put(zipCompressedData.data(), zipCompressedData.size());
    builder.putInt32(metadataBytes.size());
    builder.put(metadataBytes.data(), metadataBytes.size());
    return builder.build();
}

void compressed_chunks::decode(Chunk& chunk, const ubyte* src, size_t size) {
    ByteReader reader(src, size);

    ubyte flags = reader.get();
    reader.skip(1); // Reserved byte

    if (flags & HAS_VOXELS) {
        size_t zipCompressedSize = reader.getInt32();

        auto rleData = zip::decompress(reader.pointer(), zipCompressedSize);
        reader.skip(zipCompressedSize);

        static util::Buffer<ubyte> voxelData (CHUNK_DATA_LEN);
        extrle::decode16(rleData.data(), rleData.size(), voxelData.data());
        chunk.decode(voxelData.data());
        chunk.updateHeights();
    }
    if (flags & HAS_METADATA) {
        size_t metadataSize = reader.getInt32();
        chunk.blocksMetadata.deserialize(reader.pointer(), metadataSize);
        reader.skip(metadataSize);
    }
    chunk.setModifiedAndUnsaved();
}
