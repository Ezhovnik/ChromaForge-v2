#ifndef ASSETS_ASSETS_LOADER_H_
#define ASSETS_ASSETS_LOADER_H_

#include <string>
#include <functional>
#include <map>
#include <queue>
#include <filesystem>

/**
 * @brief Типы ресурсов, которые могут быть загружены через AssetsLoader.
 */
enum class AssetType {
     Texture, ///< Текстура
	Shader, ///< Шейдерная программа
	Font, ///< Растровый шрифт
	Atlas, ///< Атлас текстур
	Layout, ///< Макет интерфейса (XML)
     Sound ///< Звук
};

class Assets;
class ResPaths;
class Content;
class AssetsLoader;

struct AssetsConfig {
	virtual ~AssetsConfig() {}
};

/**
 * @brief Структура для передачи конфигурации при загрузке макетов интерфейса.
 */
struct LayoutConfig : AssetsConfig {
     int env; ///< Идентификатор окружения
     LayoutConfig(int env) : env(env) {}
};

struct SoundConfig : AssetsConfig {
	bool keepPCM;
	SoundConfig(bool keepPCM) : keepPCM(keepPCM) {}
};

/**
 * @brief Тип функции-загрузчика конкретного ресурса.
 * 
 * @param loader Ссылка на загрузчик (может использоваться для загрузки вложенных ресурсов).
 * @param assets Указатель на менеджер ресурсов, куда сохранять результат.
 * @param paths Указатель на объект с путями для поиска файлов.
 * @param filename Имя файла или базовое имя (интерпретация зависит от конкретного загрузчика).
 * @param alias Имя, под которым ресурс будет сохранён в Assets.
 * @param config Дополнительная конфигурация (может быть специфичной для типа ресурса).
 * @return true при успешной загрузке, false при ошибке.
 */
using aloader_func = std::function<bool(AssetsLoader&, Assets*, const ResPaths*, const std::string&, const std::string&, std::shared_ptr<AssetsConfig>)>;

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
	Assets* assets; ///< Менеджер ресурсов, куда сохраняются загруженные объекты
	std::map<AssetType, aloader_func> loaders; ///< Зарегистрированные загрузчики по типам
	std::queue<aloader_entry> entries; ///< Очередь заданий на загрузку

	const ResPaths* paths; ///< Пути для поиска файлов
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
		const std::string filename, 
		const std::string alias,
		std::shared_ptr<AssetsConfig> config=nullptr
	);

	/**
     * @brief Проверяет, есть ли ещё задания в очереди.
     * @return true, если очередь не пуста.
     */
	bool hasNext() const;

	/**
     * @brief Загружает следующий ресурс из очереди.
     * @return true, если загрузка выполнена успешно (даже если сам ресурс не загрузился? 
     *         Возвращаемое значение соответствует успешности вызова загрузчика, 
     *         но сам загрузчик может вернуть false при ошибке загрузки ресурса).
     *
     * @note Предполагается, что очередь не пуста (можно проверить через hasNext).
     */
	bool loadNext();

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
};

#endif // ASSETS_ASSETS_LOADER_H_
