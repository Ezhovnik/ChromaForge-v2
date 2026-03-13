#ifndef AUDIO_ALAUDIO_H_
#define AUDIO_ALAUDIO_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <queue>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <glm/glm.hpp>

#include "../audio.h"
#include "../../typedefs.h"

namespace audio {
    struct ALBuffer;
    class ALAudio;
    class PCMStream;

    class ALSound : public Sound {
    private:
        ALAudio* al;
        uint buffer;
        std::shared_ptr<PCM> pcm;
        duration_t duration;
    public:
        ALSound(ALAudio* al, uint buffer, std::shared_ptr<PCM> pcm, bool keepPCM);
        ~ALSound();

        duration_t getDuration() const override {
            return duration;
        }

        std::shared_ptr<PCM> getPCM() const override {
            return pcm;
        }

        Speaker* newInstance(Priority priority, int channel) const override;
    };

    class ALStream : public Stream {
    private:
        static inline constexpr size_t BUFFER_SIZE = 44100;

        ALAudio* al;
        std::shared_ptr<PCMStream> source;
        std::queue<uint> unusedBuffers;
        speakerid_t speaker = 0;
        bool keepSource;
        char buffer[BUFFER_SIZE];
        bool loop = false;

        bool preloadBuffer(uint buffer, bool loop);
        void unqueueBuffers(uint alsource);
        uint enqueueBuffers(uint alsource);
    public:
        size_t totalPlayedSamples = 0;

        ALStream(ALAudio* al, std::shared_ptr<PCMStream> source, bool keepSource);
        ~ALStream();

        std::shared_ptr<PCMStream> getSource() const override;
        void bindSpeaker(speakerid_t speaker) override;
        Speaker* createSpeaker(bool loop, int channel) override;
        speakerid_t getSpeaker() const override;
        void update(double delta) override;
        duration_t getTime() const override;
        void setTime(duration_t time) override;

        static inline constexpr uint STREAM_BUFFERS = 3;
    };

    class ALSpeaker : public Speaker {
    private:
        ALAudio* al;
        Priority priority;
        int channel;
        float volume = 0.0f;
    public:
        ALStream* stream = nullptr;

        uint source;
        bool stopped = true;
        bool paused = false;

        duration_t duration = 0.0f;

        ALSpeaker(ALAudio* al, uint source, Priority priority, int channel);
        ~ALSpeaker();

        void update(const Channel* channel, float masterVolume) override;
        int getChannel() const override;

        State getState() const override;

        float getVolume() const override;
        void setVolume(float volume) override;

        float getPitch() const override;
        void setPitch(float pitch) override;

        bool isLoop() const override;
        void setLoop(bool loop) override;

        void play() override;
        void pause() override;
        void stop() override;

        duration_t getTime() const override;
        duration_t getDuration() const override;
        void setTime(duration_t time) override;

        void setPosition(glm::vec3 pos) override;
        glm::vec3 getPosition() const override;

        void setVelocity(glm::vec3 vel) override;
        glm::vec3 getVelocity() const override;

        void setRelative(bool relative) override;
        bool isRelative() const override;

        Priority getPriority() const override;
    };

    class ALAudio : public Backend {
    private:
        ALCdevice* device;
        ALCcontext* context;

        std::vector<uint> allsources;
        std::vector<uint> freesources;

        std::vector<uint> allbuffers;
        std::vector<uint> freebuffers;

        uint maxSources;

        ALAudio(ALCdevice* device, ALCcontext* context);
    public:
        ~ALAudio();

        uint getFreeSource();
        uint getFreeBuffer();
        void freeSource(uint source);
        void freeBuffer(uint buffer);

        std::vector<std::string> getAvailableDevices() const;

        Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM) override;
        Stream* openStream(std::shared_ptr<PCMStream> stream, bool keepSource) override;

        void setListener(
            glm::vec3 position,
            glm::vec3 velocity,
            glm::vec3 lookAt,
            glm::vec3 up
        ) override;

        void update(double delta) override;

        bool isDummy() const override {
            return false;
        }

        static ALAudio* create();
    };
}

#endif // AUDIO_ALAUDIO_H_
