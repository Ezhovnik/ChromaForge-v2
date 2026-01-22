#ifndef SRC_ASSETS_LOADER_H
#define SRC_ASSETS_LOADER_H

#include <string>
#include <functional>
#include <map>
#include <queue>

// Перечисление типов ресурсов, поддерживаемых загрузчиком
enum class AssetsType {
    Texture,
    Shader,
    Font,
    Atlas
};

class Assets;

// Сигнатура функции-загрузчика ресурсов
typedef std::function<bool(Assets*, const std::string&, const std::string&)> aloader_func;

// Описание ресурса для загрузки
struct aloader_entry {
	AssetsType tag;
	const std::string filename;
	const std::string alias;
};

// Класс для управления загрузкой ресурсов
class AssetsLoader {
	Assets* assets;
	std::map<AssetsType, aloader_func> loaders;
	std::queue<aloader_entry> entries;
public:
	AssetsLoader(Assets* assets); // Конструктор загрузчика ресурсов
	void addLoader(AssetsType tag, aloader_func func); // Регистрирует функцию-загрузчик для указанного типа ресурсов
	void add(AssetsType tag, const std::string filename, const std::string alias); // Добавляет ресурс в очередь на загрузку

	bool hasNext() const; // Проверяет наличие ресурсов в очереди на загрузку
	bool loadNext(); // Загружает следующий ресурс из очереди

	static void createDefaults(AssetsLoader& loader); // Настраивает стандартные загрузчики для всех типов ресурсов
    static void addDefaults(AssetsLoader& loader); // Добавляет стандартные ресурсы в очередь загрузки
};

#endif // SRC_ASSETS_LOADER_H
