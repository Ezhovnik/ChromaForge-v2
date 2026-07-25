#include <coders/wav.h>

#include <vector>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>

#include <audio/audio.h>
#include <debug/Logger.h>
#include <io/io.h>

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

std::unique_ptr<audio::PCMStream> wav::create_stream(const io::path& file) {
    std::ifstream in(io::resolve(file), std::ios::binary);
    if (!in.is_open()) {
        THROW_ERR("Could not open file '{}'", file.string());
    }

    char buffer[6];
    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read RIFF");
    }
    if (std::strncmp(buffer, "RIFF", 4) != 0) {
        THROW_ERR("File is not a valid WAVE file (header doesn't begin with RIFF)");
    }

    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read size of file");
    }

    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read WAVE");
    }
    if (std::strncmp(buffer, "WAVE", 4) != 0) {
        THROW_ERR("File is not a valid WAVE file (header doesn't contain WAVE)");
    }

    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read fmt/0");
    }
    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read subchunk1 size");
    }
    if (!in.read(buffer, 2)) {
        THROW_ERR("Could not read audio format (PCM)");
    }
    if (!in.read(buffer, 2)) {
        THROW_ERR("Could not read number of channels");
    }
    int channels = convert_to_int(buffer, 2);
    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read sample rate");
    }
    int sampleRate = convert_to_int(buffer, 4);
    if (!in.read(buffer, 6)) {
        THROW_ERR("Could not read WAV header (byte rate and block align)");
    }
    if (!in.read(buffer, 2)) {
        THROW_ERR("Could not read bits per sample");
    }

    int bitsPerSample = convert_to_int(buffer, 2);
    if (bitsPerSample >= 24) {
        THROW_ERR("{} bit depth is not supported by OpenAL", bitsPerSample);
    }

    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read data chunk header");
    }

    size_t initialOffset = 44;
    if (std::strncmp(buffer, "LIST", 4) == 0) {
        if (!in.read(buffer, 4)) {
            THROW_ERR("Could not read comment chunk size");
        }
        int chunkSize = convert_to_int(buffer, 4);
        in.seekg(chunkSize, std::ios_base::cur);

        initialOffset += chunkSize + 4;

        if (!in.read(buffer, 4)) {
            THROW_ERR("Could not read comment chunk header");
        }
    }

    if (std::strncmp(buffer, "data", 4) != 0) {
        THROW_ERR("File is not a valid WAVE file (doesn't have 'data' tag)");
    }

    if (!in.read(buffer, 4)) {
        THROW_ERR("Could not read data size");
    }
    size_t size = convert_to_int(buffer, 4);

    if (in.eof()) {
        THROW_ERR("Reached EOF on the file");
    }
    if (in.fail()) {
        THROW_ERR("Fail state set on the file");
    }

    return std::make_unique<WavStream>(
        std::move(in), channels, bitsPerSample, sampleRate, size, initialOffset
    );
}

std::unique_ptr<audio::PCM> wav::load_pcm(
    const io::path& file, bool headerOnly
) {
    auto stream = wav::create_stream(file);

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
    return std::make_unique<audio::PCM>(
        std::move(data), totalSamples, channels, bitsPerSample, sampleRate, true
    );
}
