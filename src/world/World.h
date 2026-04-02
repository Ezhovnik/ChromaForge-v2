#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <filesystem>
#include <memory>

#include "../typedefs.h"
#include "../util/timeutil.h"
#include "../content/ContentPack.h"
#include "../data/dynamic.h"
#include "../interfaces/Serializable.h"

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
class ContentLUT;
struct EngineSettings;

/**
 * @brief Класс, представляющий мир: хранит имя, сид, список контент-паков,
 *        таймеры дня/ночи, а также управляет файловым вводом/выводом.
 *
 * Реализует интерфейс Serializable для сохранения и загрузки метаданных мира.
 * Также содержит ссылки на Content и настройки движка.
 */
class World : Serializable {
private:
	std::string name;  ///< Внутреннее имя мира
	uint64_t seed; ///< Сид для генерации мира

	std::string generator;

	EngineSettings& settings;
	const Content* const content;
	std::vector<ContentPack> packs; ///< Список контент-паков, установленных в мире

	int64_t nextInventoryId = 0; ///< Счётчик для выдачи следующих идентификаторов инвентарей
public:
	std::unique_ptr<WorldFiles> wfile; ///< Менеджер файлов мира

	/** 
     * Таймер дня/ночи в диапазоне 0..1.
     * 0.0 — полночь, 0.5 — полдень.
     */
	float daytime = timeutil::time_value(10, 00, 00);

	/// Скорость течения времени
	float daytimeSpeed = 1.0f / 60.0f / 24.0f;

	/// Общее прошедшее время в секундах с момента создания мира
	double totalTime = 0.0;

     float skyClearness = 0.0f;

	/**
     * @brief Конструктор.
     * @param name Внутреннее имя мира.
	* @param generator Идентификатор генератора мира (тип мира).
     * @param directory Корневая папка мира (используется для создания WorldFiles).
     * @param seed Сид генерации.
     * @param settings Ссылка на настройки.
     * @param content Контент.
     * @param packs Список контент-паков мира.
     */
	World(
		std::string name,
		std::string generator,
		const std::filesystem::path& directory,
		uint64_t seed,
		EngineSettings& settings,
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
     * при несовпадении контента. Если требуется, создаёт ContentLUT.
     * @param directory Папка мира.
     * @param content Текущий контент.
     * @return Указатель на ContentLUT, если требуется конвертация, иначе nullptr.
     */
	static std::shared_ptr<ContentLUT> checkIndices(
          const std::filesystem::path& directory, const Content* content
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
     * @param directory Папка мира.
     * @param settings Настройки.
     * @param content Контент (должен соответствовать сохранённому).
     * @param packs Список паков (будет сверен с сохранённым).
     * @return Указатель на Level, содержащий загруженный World.
     * @throws world_load_error Если world.json не найден или повреждён.
     */
     static std::unique_ptr<Level> load(
		const std::filesystem::path& directory, 
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

	std::unique_ptr<dynamic::Map> serialize() const override;
    void deserialize(dynamic::Map *src) override;

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
          return nextInventoryId++;
     }
};

#endif // WORLD_WORLD_H_
