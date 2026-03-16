#include "wav.h"

#include <vector>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>

#include "../audio/audio.h"
#include "../debug/Logger.h"

bool is_big_endian() {
    uint32_t ui32_v = 0x01020304;
    char bytes[sizeof(uint32_t)];
    std::memcpy(bytes, &ui32_v, sizeof(uint32_t));
    return bytes[0] == 1;
}

std::int32_t convert_to_int(char* buffer, std::size_t len){
    std::int32_t a = 0;
    if (!is_big_endian()) {
        std::memcpy(&a, buffer, len);
    } else {
        for (std::size_t i = 0; i < len; ++i) {
            reinterpret_cast<char*>(&a)[3 - i] = buffer[i];
        }
    }
    return a;
}

class WavStream : public audio::PCMStream {
private:
    std::ifstream in;
    uint channels;
    uint bytesPerSample;
    uint sampleRate;
    size_t totalSize;
    size_t totalSamples;
    size_t initialPosition;
public:
    WavStream(
        std::ifstream in, 
        uint channels, 
        uint bitsPerSample,
        uint sampleRate,
        size_t size,
        size_t initialPosition
    ) : in(std::move(in)), 
        channels(channels), 
        bytesPerSample(bitsPerSample / 8),
        sampleRate(sampleRate),
        totalSize(size) 
    {
        totalSamples = totalSize / channels / bytesPerSample;
        this->initialPosition = initialPosition;
    }

    size_t read(char* buffer, size_t bufferSize) override {
        if (!isOpen()) return 0;
        in.read(buffer, bufferSize);
        if (in.eof()) return 0;
        if (in.fail()) {
            LOG_ERROR("I/O error ocurred");
            return -1;
        }
        return in.gcount();
    }
    
    void close() override {
        if (!isOpen()) return;
        in.close();
    }

    bool isOpen() const override {
        return in.is_open();
    }

    size_t getTotalSamples() const override {
        return totalSamples;
    }

    audio::duration_t getTotalDuration() const override {
        return totalSamples / static_cast<audio::duration_t>(sampleRate);
    }

    uint getChannels() const override {
        return channels;
    }

    uint getSampleRate() const override {
        return sampleRate;
    }

    uint getBitsPerSample() const override {
        return bytesPerSample * 8;
    }

    bool isSeekable() const override {
        return true;
    }

    void seek(size_t position) override {
        if (!isOpen()) return;
        position %= totalSamples;
        in.clear();
        in.seekg(initialPosition + position * channels * bytesPerSample, std::ios_base::beg);
    }
};

audio::PCMStream* wav::create_stream(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in.is_open()) {
        LOG_ERROR("Could not open file '{}'", file.u8string());
        throw std::runtime_error("Could not open file '" + file.u8string() + "'");
    }

    char buffer[6];
    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read RIFF");
        throw std::runtime_error("Could not read RIFF");
    }
    if (std::strncmp(buffer, "RIFF", 4) != 0) {
        LOG_ERROR("File is not a valid WAVE file (header doesn't begin with RIFF)");
        throw std::runtime_error("File is not a valid WAVE file (header doesn't begin with RIFF)");
    }

    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read size of file");
        throw std::runtime_error("Could not read size of file");
    }

    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read WAVE");
        throw std::runtime_error("Could not read WAVE");
    }
    if (std::strncmp(buffer, "WAVE", 4) != 0) {
        LOG_ERROR("File is not a valid WAVE file (header doesn't contain WAVE)");
        throw std::runtime_error("File is not a valid WAVE file (header doesn't contain WAVE)");
    }

    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read fmt/0");
        throw std::runtime_error("Could not read fmt/0");
    }
    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read subchunk1 size");
        throw std::runtime_error("Could not read subchunk1 size");
    }
    if (!in.read(buffer, 2)) {
        LOG_ERROR("Could not read audio format (PCM)");
        throw std::runtime_error("Could not read audio format (PCM)");
    }
    if (!in.read(buffer, 2)) {
        LOG_ERROR("Could not read number of channels");
        throw std::runtime_error("Could not read number of channels");
    }
    int channels = convert_to_int(buffer, 2);
    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read sample rate");
        throw std::runtime_error("Could not read sample rate");
    }
    int sampleRate = convert_to_int(buffer, 4);
    if (!in.read(buffer, 6)) {
        LOG_ERROR("Could not read WAV header (byte rate and block align)");
        throw std::runtime_error("Could not read WAV header (byte rate and block align)");
    }
    if (!in.read(buffer, 2)) {
        LOG_ERROR("Could not read bits per sample");
        throw std::runtime_error("Could not read bits per sample");
    }

    int bitsPerSample = convert_to_int(buffer, 2);
    if (bitsPerSample >= 24) {
        LOG_ERROR("{} bit depth is not supported by OpenAL", bitsPerSample);
        throw std::runtime_error(std::to_string(bitsPerSample) + " bit depth is not supported by OpenAL");
    }

    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read data chunk header");
        throw std::runtime_error("Could not read data chunk header");
    }

    size_t initialOffset = 44;
    if (std::strncmp(buffer, "LIST", 4) == 0) {
        if (!in.read(buffer, 4)) {
            LOG_ERROR("Could not read comment chunk size");
            throw std::runtime_error("could not read comment chunk size");
        }
        int chunkSize = convert_to_int(buffer, 4);
        in.seekg(chunkSize, std::ios_base::cur);

        initialOffset += chunkSize + 4;

        if (!in.read(buffer, 4)) {
            LOG_ERROR("Could not read comment chunk header");
            throw std::runtime_error("could not read data chunk header");
        }
    }

    if (std::strncmp(buffer, "data", 4) != 0) {
        LOG_ERROR("File is not a valid WAVE file (doesn't have 'data' tag). Buffer: {}", buffer);
        throw std::runtime_error("File is not a valid WAVE file (doesn't have 'data' tag)");
    }

    if (!in.read(buffer, 4)) {
        LOG_ERROR("Could not read data size");
        throw std::runtime_error("Could not read data size");
    }
    size_t size = convert_to_int(buffer, 4);

    if (in.eof()) {
        LOG_ERROR("Reached EOF on the file");
        throw std::runtime_error("Reached EOF on the file");
    }
    if (in.fail()) {
        LOG_ERROR("Fail state set on the file");
        throw std::runtime_error("Fail state set on the file");
    }

    return new WavStream(std::move(in), channels, bitsPerSample, sampleRate, size, initialOffset);
}

audio::PCM* wav::load_pcm(const std::filesystem::path& file, bool headerOnly) {
    std::unique_ptr<audio::PCMStream> stream(wav::create_stream(file));

    size_t totalSamples = stream->getTotalSamples();
    uint channels = stream->getChannels();
    uint bitsPerSample = stream->getBitsPerSample();
    uint sampleRate = stream->getSampleRate();

    std::vector<char> data;
    if (!headerOnly) {
        size_t size = stream->getTotalSamples() * (stream->getBitsPerSample() / 8) * stream->getChannels();
        data.resize(size);
        stream->readFully(data.data(), size, false);
    }
    return new audio::PCM(std::move(data), totalSamples, channels, bitsPerSample, sampleRate, true);
}
