#pragma once

#include <vector>
#include <stdexcept>
#include <functional>
#include <glm/glm.hpp>

namespace util {
    template<class T, typename TCoord=int>
    class AreaMap2D {
    public:
        using OutCallback = std::function<void(TCoord, TCoord, const T&)>;
    private:
        TCoord offsetX = 0, offsetZ = 0;
        TCoord sizeX, sizeZ;
        std::vector<T> firstBuffer;
        std::vector<T> secondBuffer;
        OutCallback outCallback;

        size_t valuesCount = 0;

        void translate(TCoord dx, TCoord dz) {
            if (dx == 0 && dz == 0) {
                return;
            }
            std::fill(secondBuffer.begin(), secondBuffer.end(), T{});
            for (TCoord z = 0; z < sizeZ; ++z) {
                for (TCoord x = 0; x < sizeX; ++x) {
                    auto value = std::move(firstBuffer[z * sizeX + x]);
                    auto nx = x - dx;
                    auto nz = z - dz;
                    if (value == T{}) {
                        continue;
                    }
                    if (nx < 0 || nz < 0 || nx >= sizeX || nz >= sizeZ) {
                        if (outCallback) {
                            outCallback(x + offsetX, z + offsetZ, value);
                        }
                        valuesCount--;
                        continue;
                    }
                    secondBuffer[nz * sizeX + nx] = std::move(value);
                }
            }
            std::swap(firstBuffer, secondBuffer);
            offsetX += dx;
            offsetZ += dz;
        }
    public:
        AreaMap2D(TCoord width, TCoord height)
            : sizeX(width), sizeZ(height), 
              firstBuffer(width * height), secondBuffer(width * height) {
        }

        const T* getIf(TCoord x, TCoord z) const {
            auto lx = x - offsetX;
            auto lz = z - offsetZ;
            if (lx < 0 || lz < 0 || lx >= sizeX || lz >= sizeZ) {
                return nullptr;
            }
            return &firstBuffer[lz * sizeX + lx];
        }

        T get(TCoord x, TCoord z) const {
            auto lx = x - offsetX;
            auto lz = z - offsetZ;
            if (lx < 0 || lz < 0 || lx >= sizeX || lz >= sizeZ) {
                return T{};
            }
            return firstBuffer[lz * sizeX + lx];
        }

        T get(TCoord x, TCoord z, const T& def) const {
            if (auto ptr = getIf(x, z)) {
                const auto& value = *ptr;
                if (value == T{}) {
                    return def;
                }
                return value;
            }
            return def;
        }

        bool isInside(TCoord x, TCoord z) const {
            auto lx = x - offsetX;
            auto lz = z - offsetZ;
            return !(lx < 0 || lz < 0 || lx >= sizeX || lz >= sizeZ);
        }

        const T& require(TCoord x, TCoord z) const {
            auto lx = x - offsetX;
            auto lz = z - offsetZ;
            if (lx < 0 || lz < 0 || lx >= sizeX || lz >= sizeZ) {
                throw std::invalid_argument("Position is out of window");
            }
            return firstBuffer[lz * sizeX + lx];
        }

        bool set(TCoord x, TCoord z, T value) {
            auto lx = x - offsetX;
            auto lz = z - offsetZ;
            if (lx < 0 || lz < 0 || lx >= sizeX || lz >= sizeZ) {
                return false;
            }
            auto& element = firstBuffer[lz * sizeX + lx];
            if (value && !element) {
                valuesCount++;
            }
            if (element && !value) {
                valuesCount--;
            }
            element = std::move(value);
            return true;
        }

        void remove(TCoord x, TCoord z) {
            auto lx = x - offsetX;
            auto lz = z - offsetZ;
            if (lx < 0 || lz < 0 || lx >= sizeX || lz >= sizeZ) {
                return;
            }
            if (outCallback) outCallback(x, z, firstBuffer[lz * sizeX + lx]);
            firstBuffer[lz * sizeX + lx] = T{};
        }

        void setOutCallback(const OutCallback& callback) {
            outCallback = callback;
        }

        void resize(TCoord newSizeX, TCoord newSizeZ) {
            if (newSizeX < sizeX) {
                TCoord delta = sizeX - newSizeX;
                translate(delta / 2, 0);
                translate(-delta, 0);
                translate(delta, 0);
            }
            if (newSizeZ < sizeZ) {
                TCoord delta = sizeZ - newSizeZ;
                translate(0, delta / 2);
                translate(0, -delta);
                translate(0, delta);
            }
            const TCoord newVolume = newSizeX * newSizeZ;
            std::vector<T> newFirstBuffer(newVolume);
            std::vector<T> newSecondBuffer(newVolume);
            for (TCoord z = 0; z < sizeZ && z < newSizeZ; ++z) {
                for (TCoord x = 0; x < sizeX && x < newSizeX; ++x) {
                    newFirstBuffer[z * newSizeX + x] = firstBuffer[z * sizeX + x];
                }
            }
            sizeX = newSizeX;
            sizeZ = newSizeZ;
            firstBuffer = std::move(newFirstBuffer);
            secondBuffer = std::move(newSecondBuffer);
        }

        void setCenter(TCoord centerX, TCoord centerZ) {
            auto deltaX = centerX - (offsetX + sizeX / 2);
            auto deltaZ = centerZ - (offsetZ + sizeZ / 2);
            if (deltaX | deltaZ) {
                translate(deltaX, deltaZ);
            }
        }

        void clear() {
            for (TCoord z = 0; z < sizeZ; ++z) {
                for (TCoord x = 0; x < sizeX; ++x) {
                    auto i = z * sizeX + x;
                    auto value = firstBuffer[i];
                    firstBuffer[i] = {};
                    if (outCallback && value != T{}) {
                        outCallback(x + offsetX, z + offsetZ, value);
                    }
                }
            }
            valuesCount = 0;
        }

        TCoord getOffsetX() const {
            return offsetX;
        }

        TCoord getOffsetZ() const {
            return offsetZ;
        }

        TCoord getWidth() const {
            return sizeX;
        }

        TCoord getDepth() const {
            return sizeZ;
        }

        const std::vector<T>& getBuffer() const {
            return firstBuffer;
        }

        size_t count() const {
            return valuesCount;
        }

        TCoord area() const {
            return sizeX * sizeZ;
        }
    };
}
