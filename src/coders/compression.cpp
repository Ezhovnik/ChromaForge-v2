#include <coders/compression.h>

#include <cstring>
#include <stdexcept>
#include <string>

#include <coders/rle.h>
#include <coders/zip.h>
#include <util/BufferPool.h>
#include <debug/Logger.h>

using namespace compression;

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
    auto buffer = get_buffer(srclen * 2);
    auto bytes = buffer.get();
    std::unique_ptr<ubyte[]> uptr;
    if (bytes == nullptr) {
        uptr = std::make_unique<ubyte[]>(srclen * 2);
        bytes = uptr.get();
    }
    len = encodefunc(src, srclen, bytes);
    if (uptr) {
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
            LOG_ERROR("Not implemented");
            throw std::runtime_error("Not implemented");
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
                LOG_ERROR("Expected decompressed size {} got {}", dstlen, decoded);
                throw std::runtime_error(
                    "Expected decompressed size " + std::to_string(dstlen) + " got " + std::to_string(decoded)
                );
            }
            return decompressed;
        }
        case Method::Zip: {
            auto buffer = zip::decompress(src, srclen);
            if (buffer.size() != dstlen) {
                LOG_ERROR("Expected decompressed size {} got {}", dstlen, buffer.size());
                throw std::runtime_error(
                    "Expected decompressed size " + std::to_string(dstlen) + " got " + std::to_string(buffer.size())
                );
            }
            auto decompressed = std::make_unique<ubyte[]>(buffer.size());
            std::memcpy(decompressed.get(), buffer.data(), buffer.size());
            return decompressed;
        }
        default:
            LOG_ERROR("Not implemented");
            throw std::runtime_error("Not implemented");
    }
}
