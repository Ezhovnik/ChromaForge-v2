#pragma once

#include <string>
#include <filesystem>
#include <memory>

#include <typedefs.h>
#include <util/timeutil.h>
#include <content/ContentPack.h>
#include <data/dynamic.h>
#include <interfaces/Serializable.h>

/**
 * @brief Исключение, выбрасываемое при ошибках загрузки мира.
 */
class world_load_error : public std::runtime_error {
public:
	world_load_error(const std::string& message);
};

class WorldFiles;
class Level;
class Content;
class ContentReport;
struct EngineSettings;

struct WorldInfo : public Serializable {
	std::string name;  ///< Внутреннее имя мира
	uint64_t seed; ///< Сид для генерации мира

	std::string generator;

	int64_t nextInventoryId = 0; ///< Счётчик для выдачи следующих идентификаторов инвентарей

	/** 
     * Таймер дня/ночи в диапазоне 0..1.
     * 0.0 — полночь, 0.5 — полдень.
     */
	float daytime = timeutil::time_value(10, 00, 00);

	/// Скорость течения времени
	float daytimeSpeed = 1.0f;

	/// Общее прошедшее время в секундах с момента создания мира
	double totalTime = 0.0;

     float skyClearness = 0.0f;

     entityid_t nextEntityId = 0;

     int major = 0, minor = -1, maintenance = -1;

     std::unique_ptr<dynamic::Map> serialize() const override;
     void deserialize(dynamic::Map* src) override;
};

class World {
     WorldInfo info {};

     const Content* const content;
     std::vector<ContentPack> packs;

     // int64_t nextInventoryId = 0;

     void writeResources(const Content* content);
public:
     std::shared_ptr<WorldFiles> wfile;

	World(
		WorldInfo info,
          const std::shared_ptr<WorldFiles>& worldFiles,
		const Content* content, 
		const std::vector<ContentPack>& packs
	);
	~World();

	/** 
     * Обновляет таймеры мира (время суток и общее время).
     * @param delta Изменение времени в секундах.
     */
	void updateTimers(float delta);

	/**
     * Сохраняет все несохранённые данные мира на диск.
     * @param level Уровень, содержащий чанки и игрока.
     */
     void write(Level* level);

	/** 
     * Проверяет необходимость конвертации индексов блоков/предметов
     * при несовпадении контента. Если требуется, создаёт ContentReport.
     * @param directory Папка мира.
     * @param content Текущий контент.
     * @return Указатель на ContentReport, если требуется конвертация, иначе nullptr.
     */
	static std::shared_ptr<ContentReport> checkIndices(
          const std::shared_ptr<WorldFiles>& worldFiles, const Content* content
     );

	/**
     * Создаёт новый мир (генерация с нуля).
     * @param name Имя мира.
	* @param generator Идентификатор генератора мира (тип мира).
     * @param directory Папка для сохранения.
     * @param seed Сид.
     * @param settings Настройки.
     * @param content Контент.
     * @param packs Список паков.
     * @return Указатель на новый Level, содержащий созданный World.
     */
	static std::unique_ptr<Level> create(
		const std::string& name,
		const std::string& generator,
		const std::filesystem::path& directory, 
		uint64_t seed, 
		EngineSettings& settings, 
		const Content* content,
		const std::vector<ContentPack>& packs
	);

	/**
     * Загружает существующий мир из папки.
     * @param worldFiles Файловый менеджер мира.
     * @param settings Настройки.
     * @param content Контент (должен соответствовать сохранённому).
     * @param packs Список паков (будет сверен с сохранённым).
     * @return Указатель на Level, содержащий загруженный World.
     * @throws world_load_error Если world.json не найден или повреждён.
     */
     static std::unique_ptr<Level> load(
		const std::shared_ptr<WorldFiles>& worldFiles, 
		EngineSettings& settings, 
		const Content* content, 
		const std::vector<ContentPack>& packs
	);

	/**
     * @brief Устанавливает внутреннее имя мира.
     * @param name Новое имя.
     */
	void setName(const std::string& name);

	/**
     * @brief Устанавливает сид мира.
     * @param seed Новый сид.
     */
     void setSeed(uint64_t seed);

	/**
     * @brief Устанавливает идентификатор генератора мира (тип мира).
	 * @param generator Идентификатор нового генератора.
     */
	void setGenerator(const std::string& generator);

	/**
     * Возвращает внутреннее имя мира.
     */
     std::string getName() const;

	/** Возвращает сид генерации мира. */
     uint64_t getSeed() const;

	/** Возвращает идентификатор генератора мира. */
	std::string getGenerator() const;

     WorldInfo& getInfo() {
          return info;
     }

     const WorldInfo& getInfo() const {
          return info;
     }

	/** 
     * Проверяет, установлен ли в мире указанный контент-пак.
     * @param id Идентификатор пака.
     * @return true если пак есть в списке.
     */
	bool hasPack(const std::string& id) const;

	/**
     * Возвращает список контент-паков мира.
     */
	const std::vector<ContentPack>& getPacks() const;

	/**
     * Возвращает указатель на контент, используемый миром.
     */
	const Content* getContent() const {
          return content;
     }

	/**
     * Генерирует и возвращает следующий идентификатор инвентаря.
     * @return Целое число >= 1.
     */
     int64_t getNextInventoryId() {
          return info.nextInventoryId++;
     }
};
