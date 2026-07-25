#include <coders/compression.h>

#include <cstring>
#include <stdexcept>
#include <string>

#include <coders/rle.h>
#include <coders/zip.h>
#include <util/BufferPool.h>
#include <debug/Logger.h>

using namespace compression;

inline constexpr float BUFFER_NOCROP_THRESOLD = 0.9;

static util::BufferPool<ubyte> buffer_pools[] {
    {255},
    {UINT16_MAX},
    {UINT16_MAX * 8},
};

static std::shared_ptr<ubyte[]> get_buffer(size_t minSize) {
    for (auto& pool : buffer_pools) {
        if (minSize <= pool.getBufferSize()) {
            return pool.get();
        }
    }
    return nullptr;
}

static auto compress_rle(
    const ubyte* src,
    size_t srclen,
    size_t& len,
    size_t(*encodefunc)(const ubyte*, size_t, ubyte*)
) {
    size_t bufferSize = srclen * 2;
    auto buffer = get_buffer(bufferSize);
    auto bytes = buffer.get();
    std::unique_ptr<ubyte[]> uptr;
    if (bytes == nullptr) {
        uptr = std::make_unique<ubyte[]>(bufferSize);
        bytes = uptr.get();
    }
    len = encodefunc(src, srclen, bytes);
    if (uptr) {
        if (len < bufferSize * BUFFER_NOCROP_THRESOLD) {
            auto cropped = std::make_unique<ubyte[]>(len);
            std::memcpy(cropped.get(), uptr.get(), len);
            return cropped;
        }
        return uptr;
    }
    auto data = std::make_unique<ubyte[]>(len);
    std::memcpy(data.get(), bytes, len);
    return data;
}

std::unique_ptr<ubyte[]> compression::compress(
    const ubyte* src, size_t srclen, size_t& len, Method method
) {
    switch (method) {
        case Method::None:
            LOG_ERROR("Compression method is None");
            throw std::invalid_argument("Compression method is None");
        case Method::Extrle8:
            return compress_rle(src, srclen, len, extrle::encode);
        case Method::Extrle16:
            return compress_rle(src, srclen, len, extrle::encode16);
        case Method::Zip: {
            auto buffer = zip::compress(src, srclen);
            auto data = std::make_unique<ubyte[]>(buffer.size());
            std::memcpy(data.get(), buffer.data(), buffer.size());
            len = buffer.size();
            return data;
        }
        default:
            THROW_ERR("Not implemented");
    }
}

std::unique_ptr<ubyte[]> compression::decompress(
    const ubyte* src, size_t srclen, size_t dstlen, Method method
) {
    switch (method) {
        case Method::None:
            LOG_ERROR("Compression method is None");
            throw std::invalid_argument("Compression method is None");
        case Method::Extrle8: {
            auto decompressed = std::make_unique<ubyte[]>(dstlen);
            extrle::decode(src, srclen, decompressed.get());
            return decompressed;
        }
        case Method::Extrle16: {
            auto decompressed = std::make_unique<ubyte[]>(dstlen);
            size_t decoded = extrle::decode16(src, srclen, decompressed.get());
            if (decoded != dstlen) {
                THROW_ERR("Expected decompressed size {} got {}", dstlen, decoded);
            }
            return decompressed;
        }
        case Method::Zip: {
            auto buffer = zip::decompress(src, srclen);
            if (buffer.size() != dstlen) {
                THROW_ERR("Expected decompressed size {} got {}", dstlen, buffer.size());
            }
            auto decompressed = std::make_unique<ubyte[]>(buffer.size());
            std::memcpy(decompressed.get(), buffer.data(), buffer.size());
            return decompressed;
        }
        default:
            THROW_ERR("Not implemented");
    }
}
