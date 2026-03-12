#ifndef AUDIO_AUDIO_H_
#define AUDIO_AUDIO_H_

#include <vector>
#include <memory>
#include <filesystem>

#include <glm/glm.hpp>

#include "../typedefs.h"

namespace audio {
    using speakerid_t = int64_t;

    /// Единица измерения продолжительности времени - секунда.
    using duration_t = double;

    enum class Priority {
        Low = 0,
        Normal = 5,
        High = 10
    };

    class Speaker;

    enum class State {
        Playing,
        Paused,
        Stopped
    };

    /**
     * @brief Структура, хранящая данные импульсно-кодовой модуляции (PCM).
     * 
     * Содержит сырые аудиоданные, а также информацию о формате: количество каналов,
     * бит на сэмпл, частоту дискретизации.
     */
    struct PCM {
        /** 
         * Сырые аудиоданные. Могут содержать 8- или 16-битные сэмплы.
         */
        std::vector<char> data;
        size_t totalSamples;
        uint8_t channels; ///< Количество каналов (1 — моно, 2 — стерео и т.д.)
        uint8_t bitsPerSample;
        uint sampleRate; ///< Частота дискретизации в Гц
        bool seekable;

        PCM(std::vector<char> data,
            size_t totalSamples,
            uint8_t channels,
            uint8_t bitsPerSample,
            uint sampleRate,
            bool seekable
        ) : data(std::move(data)),
            totalSamples(totalSamples),
            channels(channels), 
            bitsPerSample(bitsPerSample),
            sampleRate(sampleRate),
            seekable(seekable) {}

        inline size_t countSamplesMono() const {
            return totalSamples / channels;
        }

        /**
         * @brief Вычисляет длительность звука в секундах.
         * @return Длительность, секунд.
         */
        inline duration_t getDuration() const {
            return static_cast<duration_t>(countSamplesMono()) / static_cast<duration_t>(sampleRate);
        }
    };

    class PCMStream {
    public:
        virtual ~PCMStream() {};
        virtual size_t readFully(char* buffer, size_t bufferSize, bool loop);
        virtual size_t read(char* buffer, size_t bufferSize) = 0;
        virtual void close()=0;

        virtual bool isOpen() const=0;
        virtual size_t getTotalSamples() const=0;
        virtual duration_t getTotalDuration() const=0;
        virtual uint getChannels() const=0;
        virtual uint getSampleRate() const=0;
        virtual uint getBitsPerSample() const=0;
        virtual bool isSeekable() const=0;
        virtual void seek(size_t position) = 0;
    };

    class Stream {
    public:
        virtual ~Stream() {};

        virtual std::shared_ptr<PCMStream> getSource() const = 0;

        virtual Speaker* createSpeaker(bool loop) = 0;

        virtual void bindSpeaker(speakerid_t speaker) = 0;

        virtual speakerid_t getSpeaker() const = 0;

        virtual void update(double delta) = 0;

        virtual void setTime(duration_t time) = 0;
    };

    /**
     * @brief Аудиоресурс, поддерживающий одновременное воспроизведение нескольких экземпляров.
     *
     * Хранит аудиоданные в памяти и позволяет создавать независимые источники звука.
     */
    class Sound {
    public:
        virtual ~Sound() {}

        /**
         * @brief Возвращает продолжительность звука.
         * @return Продолжительность в секундах
         */
        virtual duration_t getDuration() const = 0;

        /**
         * @brief Возвращает PCM-данные звука
         * @return Умный указатель на PCM или nullptr
         */
        virtual std::shared_ptr<PCM> getPCM() const = 0;

        /**
         * @brief Создаёт новый звуковой экземпляр
         * @param priority Приоритет экземпляра. Экземпляр с высоким
         * приоритетом может забрать динамик у экземпляра с низким.
         * @return Указатель на динамик, с которым связан новый экземпляр,
         * или nullptr в случае неудачи
         */
        virtual Speaker* newInstance(Priority priority) const = 0;
    };

    /**
     * @brief Интерфейс контроллера источника звука (динамика).
     *
     * Предоставляет управление звуковым источником: воспроизведение, пауза,
     * остановка, регулировка громкости, высоты тона и позиционирование в 3D.
     */
    class Speaker {
    public:
        virtual ~Speaker() {}

        /**
         * @brief Возвращает текущее состояние динамика.
         * @return Состояние динамика (например, Playing, Paused, Stopped).
         */
        virtual State getState() const = 0;

        /**
         * @brief Возвращает текущую громкость динамика.
         * @return Значение громкости.
         */
        virtual float getVolume() const = 0;

        /**
         * @brief Устанавливает громкость динамика.
         * @param volume Новое значение громкости (должно быть неотрицательным).
         */
        virtual void setVolume(float volume) = 0;

        /**
         * @brief Возвращает множитель высоты тона (pitch).
         * @return Текущий множитель.
         */
        virtual float getPitch() const = 0;

        /**
         * @brief Устанавливает множитель высоты тона.
         * @param pitch Новый множитель (должен быть положительным; 1.0 — норма).
         */
        virtual void setPitch(float pitch) = 0;

        virtual bool isLoop() const = 0;

        virtual void setLoop(bool loop) = 0;

        /**
         * @brief Запускает или возобновляет воспроизведение.
         */
        virtual void play() = 0;

        /**
         * @brief Приостанавливает воспроизведение (динамик остаётся активным).
         */
        virtual void pause() = 0;

        /**
         * @brief Останавливает и уничтожает динамик.
         */
        virtual void stop() = 0;

        virtual bool isStoppedManually() const = 0;

        /**
         * @brief Возвращает текущую позицию воспроизведения.
         * @return Время в секундах.
         */
        virtual duration_t getTime() const = 0;

        /**
         * @brief Устанавливает позицию воспроизведения.
         * @param time Новая позиция в секундах.
         */
        virtual void setTime(duration_t time) = 0;

        /**
         * @brief Устанавливает позицию динамика в 3D-мире.
         * @param pos Вектор новой позиции.
         */
        virtual void setPosition(glm::vec3 pos) = 0;

        /**
         * @brief Возвращает текущую позицию динамика.
         * @return Вектор позиции.
         */
        virtual glm::vec3 getPosition() const = 0;

        /**
         * @brief Устанавливает скорость динамика (для эффекта Доплера).
         * @param vel Вектор скорости.
         */
        virtual void setVelocity(glm::vec3 vel) = 0;

        /**
         * @brief Возвращает текущую скорость динамика.
         * @return Вектор скорости.
         */
        virtual glm::vec3 getVelocity() const = 0;

        /**
         * @brief Возвращает приоритет динамика.
         * @return Значение приоритета.
         */
        virtual Priority getPriority() const = 0;

        virtual void setRelative(bool relative) = 0;

        virtual bool isRelative() const = 0;

        inline bool isPlaying() const {
            return getState() == State::Playing;
        }

        inline bool isPaused() const {
            return getState() == State::Paused;
        }

        inline bool isStopped() const {
            return getState() == State::Stopped;
        }
    };

    class Backend {
    public:
        virtual ~Backend() {};

        virtual Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM) = 0;

        virtual Stream* openStream(std::shared_ptr<PCMStream> stream, bool keepSource) = 0;

        /**
         * @brief Устанавливает параметры слушателя (позиция, скорость, ориентация).
         * @param position Позиция слушателя в мировых координатах.
         * @param velocity Вектор скорости слушателя (для эффекта Доплера).
         * @param lookAt Направление «взгляда» слушателя (нормализованный вектор).
         * @param up Вектор «верха» слушателя.
         */
        virtual void setListener(
            glm::vec3 position, 
            glm::vec3 velocity, 
            glm::vec3 lookAt, 
            glm::vec3 up
        ) = 0;

        virtual void update(double delta) = 0;

        virtual bool isDummy() const = 0;
    };

    /**
     * @brief Инициализация аудиосистемы
     * @param enabled Нужно ли пытаться инициализировать реальную аудиосистему
     */
    extern void initialize(bool enabled);

    extern PCM* loadPCM(const std::filesystem::path& file, bool headerOnly);

    extern Sound* loadSound(const std::filesystem::path& file, bool keepPCM);

    extern Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM);

    extern PCMStream* openPCMStream(const std::filesystem::path& file);

    extern Stream* openStream(const std::filesystem::path& file, bool keepSource);

    extern Stream* openStream(std::shared_ptr<PCMStream> stream, bool keepSource);

    /**
     * @brief Устанавливает параметры слушателя, используя текущий бэкенд.
     * @param position Позиция.
     * @param velocity Скорость.
     * @param lookAt Направление взгляда.
     * @param up Вектор верха.
     */
    extern void setListener(
        glm::vec3 position, 
        glm::vec3 velocity, 
        glm::vec3 lookAt, 
        glm::vec3 up
    );

    extern speakerid_t play(
        Sound* sound,
        glm::vec3 position,
        bool relative,
        float volume,
        float pitch,
        bool loop,
        Priority priority
    );

    extern speakerid_t play(
        std::shared_ptr<Stream> stream,
        glm::vec3 position,
        bool relative,
        float volume,
        float pitch,
        bool loop
    );

    extern speakerid_t playStream(
        const std::filesystem::path& file,
        glm::vec3 position,
        bool relative,
        float volume,
        float pitch,
        bool loop
    );

    extern Speaker* get(speakerid_t id);

    extern void update(double delta);

    /**
     * @brief Завершение работы аудиосистемы
     */
    extern void close();
};

#endif // AUDIO_AUDIO_H_
