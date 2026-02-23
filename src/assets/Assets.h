#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <unordered_map>
#include <memory>

class Texture;
class ShaderProgram;
class Font;
class Atlas;

// Менеджер для управления ресурсами
class Assets {
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
	std::unordered_map<std::string, std::shared_ptr<Atlas>> atlases;
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

	void extend(const Assets& assets);
};

#endif // ASSETS_ASSETS_H_
