#ifndef ASSETS_ASSETS_LOADER_H_
#define ASSETS_ASSETS_LOADER_H_

#include <string>
#include <functional>
#include <map>
#include <queue>
#include <filesystem>
#include <utility>

#include <assets/Assets.h>
#include <interfaces/Task.h>
#include <delegates.h>
#include <typedefs.h>

class ResPaths;
class Content;
class AssetsLoader;

namespace dynamic {
	class Map;
	class List;
}

struct AssetsConfig {
	virtual ~AssetsConfig() {}
};

/**
 * @brief Структура для передачи конфигурации при загрузке макетов интерфейса.
 */
struct LayoutConfig : AssetsConfig {
     scriptenv env; ///< Идентификатор окружения
     LayoutConfig(scriptenv env) : env(std::move(env)) {}
};

struct SoundConfig : AssetsConfig {
	bool keepPCM;
	SoundConfig(bool keepPCM) : keepPCM(keepPCM) {}
};

using aloader_func = std::function<asset_loader::postfunc(
     AssetsLoader*,
     const ResPaths*, 
     const std::string&, 
     const std::string&, 
     std::shared_ptr<AssetsConfig>
)>;

/**
 * @brief Элемент очереди загрузки.
 */
struct aloader_entry {
	AssetType tag; ///< Тип ресурса
	const std::string filename; ///< Имя файла (или путь)
	const std::string alias; ///< Псевдоним в менеджере ресурсов
	std::shared_ptr<AssetsConfig> config; ///< Дополнительные настройки
};

/**
 * @brief Асинхронный загрузчик ресурсов.
 *
 * Позволяет регистрировать функции-загрузчики для разных типов ресурсов,
 * добавлять задания в очередь и затем загружать их по одному.
 */
class AssetsLoader {
private:
	Assets* assets; ///< Менеджер ресурсов, куда сохраняются загруженные объекты
	std::map<AssetType, aloader_func> loaders; ///< Зарегистрированные загрузчики по типам
	std::queue<aloader_entry> entries; ///< Очередь заданий на загрузку

	const ResPaths* paths; ///< Пути для поиска файлов

     void tryAddSound(const std::string& name);

     void processPreload(AssetType tag, const std::string& name, dynamic::Map* map);
	void processPreloadList(AssetType tag, dynamic::List* list);
	void processPreloadConfig(const std::filesystem::path& file);
	void processPreloadConfigs(const Content* content);
public:
	/**
     * @brief Конструктор: инициализирует указатели и регистрирует стандартные загрузчики.
     * @param assets Менеджер ресурсов (не должен быть nullptr).
     * @param paths Объект с путями (не должен быть nullptr).
     */
	AssetsLoader(Assets* assets, const ResPaths* paths);

	/**
     * @brief Регистрирует функцию-загрузчик для указанного типа ресурса.
     * @param tag Тип ресурса.
     * @param func Функция загрузки.
     *
     * Если для данного типа уже был зарегистрирован загрузчик, он будет заменён.
     */
	void addLoader(AssetType tag, aloader_func func);

	/**
     * @brief Добавляет задание на загрузку в очередь.
     * @param tag Тип ресурса.
     * @param filename Имя файла (интерпретируется загрузчиком).
     * @param alias Псевдоним, под которым ресурс будет сохранён.
     * @param config Дополнительная конфигурация (по умолчанию nullptr).
     */
	void add(
		AssetType tag, 
		const std::string& filename, 
		const std::string& alias,
		std::shared_ptr<AssetsConfig> config=nullptr
	);

     std::shared_ptr<Task> startTask(runnable onDone);

	/**
     * @brief Проверяет, есть ли ещё задания в очереди.
     * @return true, если очередь не пуста.
     */
	bool hasNext() const;

	/**
     * @brief Загружает следующий ресурс из очереди.
     *
     * @note Предполагается, что очередь не пуста (можно проверить через hasNext).
     */
	void loadNext();

	/**
     * @brief Добавляет стандартные ресурсы, необходимые движку.
     * @param loader Загрузчик, в который добавляются задания.
     * @param content Указатель на контент (может быть nullptr). Если передан, добавляются также ресурсы из контента.
     */
     static void addDefaults(AssetsLoader& loader, const Content* content);

	/**
     * @brief Возвращает объект с путями, используемый загрузчиком.
     * @return Указатель на ResPaths.
     */
	const ResPaths* getPaths() const;

     aloader_func getLoader(AssetType tag);

     static bool loadExternalTexture(
          Assets* assets,
          const std::string& name,
          const std::vector<std::filesystem::path>& alternatives
     );
};

#endif // ASSETS_ASSETS_LOADER_H_
