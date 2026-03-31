#ifndef CODERS_OGG_H_
#define CODERS_OGG_H_

#include <filesystem>

namespace audio {
    struct PCM;
    class PCMStream;
}

namespace ogg {
    std::unique_ptr<audio::PCM> load_pcm(const std::filesystem::path& file, bool headerOnly);
    std::unique_ptr<audio::PCMStream> create_stream(const std::filesystem::path& file);
}

#endif // CODERS_OGG_H_
