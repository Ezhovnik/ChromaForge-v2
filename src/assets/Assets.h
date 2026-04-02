#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

#include "../graphics/core/TextureAnimation.h"

class Texture;
class ShaderProgram;
class Font;
class Atlas;
class UIDocument;
class Assets;

namespace audio {
	class Sound;
}

namespace asset_loader {
     using postfunc = std::function<void(Assets*)>;
}

/**
 * @brief Менеджер ресурсов (ассетов).
 *
 * Хранит и управляет временем жизни текстур, шейдеров, шрифтов, атласов,
 * макетов интерфейса и анимаций. Все ресурсы хранятся в виде std::shared_ptr,
 * но методы доступа возвращают сырые указатели.
 */
class Assets {
private:
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shaders;
     std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
	std::unordered_map<std::string, std::shared_ptr<Atlas>> atlases;
	std::unordered_map<std::string, std::shared_ptr<UIDocument>> layouts;
     std::unordered_map<std::string, std::shared_ptr<audio::Sound>> sounds;
	std::vector<TextureAnimation> animations; ///< Анимации, связанные с текстурами.
public:
     Assets() {}
     Assets(const Assets&) = delete;
	~Assets();

    	// --- Текстуры ---
	/**
     * @brief Получает текстуру по имени.
     * @param name Имя текстуры.
     * @return Указатель на текстуру или nullptr, если не найдена.
     */
	Texture* getTexture(const std::string& name) const;
	
	/**
     * @brief Сохраняет текстуру в менеджере.
     * @param texture Указатель на текстуру (менеджер забирает владение).
     * @param name Имя, под которым текстура будет сохранена.
     */
	void store(std::unique_ptr<Texture> texture, const std::string& name);

		// --- Шейдеры ---
     /**
     * @brief Получает шейдер по имени.
     * @param name Имя шейдера.
     * @return Указатель на шейдер или nullptr.
     */
	ShaderProgram* getShader(const std::string& name) const;

	/**
     * @brief Сохраняет шейдер в менеджере.
     * @param shader Указатель на шейдер.
     * @param name Имя для сохранения.
     */
	void store(std::unique_ptr<ShaderProgram> shader, const std::string& name);

    	// --- Шрифты ---
	/**
     * @brief Получает шрифт по имени.
     * @param name Имя шрифта.
     * @return Указатель на шрифт или nullptr.
     */
	Font* getFont(const std::string& name) const;

	/**
     * @brief Сохраняет шрифт в менеджере.
     * @param font Указатель на шрифт.
     * @param name Имя для сохранения.
     */
	void store(std::unique_ptr<Font> font, const std::string& name);

		// --- Атласы ---
	/**
     * @brief Получает атлас по имени.
     * @param name Имя атласа.
     * @return Указатель на атлас или nullptr.
     */
	Atlas* getAtlas(const std::string& name) const;

	/**
     * @brief Сохраняет атлас в менеджере.
     * @param atlas Указатель на атлас.
     * @param name Имя для сохранения.
     */
	void store(std::unique_ptr<Atlas> atlas, const std::string& name);

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
	UIDocument* getLayout(const std::string& name) const;

	/**
     * @brief Сохраняет макет в менеджере.
     * @param layout Указатель на макет.
     * @param name Имя для сохранения.
     */
	void store(std::unique_ptr<UIDocument> layout, const std::string& name);

          // --- Звуки ---
     /**
     * @brief Получает звук по имени.
     * @param name Имя звука.
     * @return Указатель на audio::Sound или nullptr.
     */
     audio::Sound* getSound(const std::string& name) const;

     /**
     * @brief Сохраняет звук в менеджере.
     * @param sound Указатель на звук.
     * @param name Имя для сохранения.
     */
	void store(std::unique_ptr<audio::Sound> sound, const std::string& name);
};

#endif // ASSETS_ASSETS_H_
