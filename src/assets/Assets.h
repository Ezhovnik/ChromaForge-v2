#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "../graphics/TextureAnimation.h"

class Texture;
class ShaderProgram;
class Font;
class Atlas;
class UIDocument;

/**
 * @brief Структура для передачи конфигурации при загрузке макетов интерфейса.
 */
struct LayoutConfig {
	int env; ///< Идентификатор окружения

	LayoutConfig(int env) : env(env) {}
};

/**
 * @brief Менеджер ресурсов (ассетов).
 *
 * Хранит и управляет временем жизни текстур, шейдеров, шрифтов, атласов,
 * макетов интерфейса и анимаций. Все ресурсы хранятся в виде std::shared_ptr,
 * но методы доступа возвращают сырые указатели.
 */
class Assets {
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
	std::unordered_map<std::string, std::shared_ptr<Atlas>> atlases;
	std::unordered_map<std::string, std::shared_ptr<UIDocument>> layouts;
	std::vector<TextureAnimation> animations; ///< Анимации, связанные с текстурами.
public:
	~Assets();

    	// --- Текстуры ---
	/**
     * @brief Получает текстуру по имени.
     * @param name Имя текстуры.
     * @return Указатель на текстуру или nullptr, если не найдена.
     */
	Texture* getTexture(std::string name) const;
	
	/**
     * @brief Сохраняет текстуру в менеджере.
     * @param texture Указатель на текстуру (менеджер забирает владение).
     * @param name Имя, под которым текстура будет сохранена.
     * @return true, если сохранение выполнено успешно, false, если имя уже занято.
     */
	bool store(Texture* texture, std::string name);

		// --- Шейдеры ---
    /**
     * @brief Получает шейдер по имени.
     * @param name Имя шейдера.
     * @return Указатель на шейдер или nullptr.
     */
	ShaderProgram* getShader(std::string name) const;

	/**
     * @brief Сохраняет шейдер в менеджере.
     * @param shader Указатель на шейдер.
     * @param name Имя для сохранения.
     * @return true при успехе, false если имя уже занято.
     */
	bool store(ShaderProgram* shader, std::string name);

    	// --- Шрифты ---
	/**
     * @brief Получает шрифт по имени.
     * @param name Имя шрифта.
     * @return Указатель на шрифт или nullptr.
     */
	Font* getFont(std::string name) const;

	/**
     * @brief Сохраняет шрифт в менеджере.
     * @param font Указатель на шрифт.
     * @param name Имя для сохранения.
     * @return true при успехе, false если имя уже занято.
     */
	bool store(Font* font, std::string name);

		// --- Атласы ---
	/**
     * @brief Получает атлас по имени.
     * @param name Имя атласа.
     * @return Указатель на атлас или nullptr.
     */
	Atlas* getAtlas(std::string name) const;

	/**
     * @brief Сохраняет атлас в менеджере.
     * @param atlas Указатель на атлас.
     * @param name Имя для сохранения.
     * @return true при успехе, false если имя уже занято.
     */
	bool store(Atlas* atlas, std::string name);

		// --- Анимации ---
	/**
     * @brief Возвращает список всех зарегистрированных анимаций.
     * @return Константная ссылка на вектор анимаций.
     */
	const std::vector<TextureAnimation>& getAnimations();

	/**
     * @brief Добавляет анимацию в менеджер.
     * @param animation Анимация (копируется).
     */
	void store(const TextureAnimation& animation);

		// --- Макеты интерфейса ---
	/**
     * @brief Получает макет по имени.
     * @param name Имя макета.
     * @return Указатель на UIDocument или nullptr.
     */
	UIDocument* getLayout(std::string name) const;

	/**
     * @brief Сохраняет макет в менеджере.
     * @param layout Указатель на макет.
     * @param name Имя для сохранения.
     * @return true при успехе, false если имя уже занято.
     */
	bool store(UIDocument* layout, std::string name);

	/**
     * @brief Расширяет текущий менеджер ресурсами из другого менеджера.
     * @param assets Другой объект Assets, из которого копируются все ресурсы.
     *
     * При совпадении имён ресурсы перезаписываются (последний имеет приоритет).
     * Анимации заменяются полностью (вектор очищается и заполняется копиями).
     */
	void extend(const Assets& assets);
};

#endif // ASSETS_ASSETS_H_
