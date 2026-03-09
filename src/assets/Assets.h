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

struct LayoutConfig {
	int env;

	LayoutConfig(int env) : env(env) {}
};

// Менеджер для управления ресурсами
class Assets {
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
	std::unordered_map<std::string, std::shared_ptr<Atlas>> atlases;
	std::unordered_map<std::string, std::shared_ptr<UIDocument>> layouts;
	std::vector<TextureAnimation> animations;
public:
	~Assets(); // Деструктор

    // Методы для работы с текстурами
	Texture* getTexture(std::string name) const; // Получает текстуру по имени
	bool store(Texture* texture, std::string name); // Сохраняет текстуру в менеджере ресурсов

    // Методы для работы с шейдерами
	ShaderProgram* getShader(std::string name) const; // Получает шейдер по имени
	bool store(ShaderProgram* shader, std::string name); // Сохраняет шейдер в менеджере ресурсов

    // Методы для работы с шрифтами
	Font* getFont(std::string name) const; // Получает шрифт по имени
	bool store(Font* font, std::string name); // Сохраняет шрифт в менеджере ресурсов

	// Методы для работы с атласами
	Atlas* getAtlas(std::string name) const; // Получает атлас по имени
	bool store(Atlas* atlas, std::string name); // Сохраняеь атлас в менеджере ресурсов

	const std::vector<TextureAnimation>& getAnimations();
	void store(const TextureAnimation& animation);

	UIDocument* getLayout(std::string name) const;
	bool store(UIDocument* layout, std::string name);

	void extend(const Assets& assets);
};

#endif // ASSETS_ASSETS_H_
