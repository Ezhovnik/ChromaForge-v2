#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <unordered_map>

class Texture;
class ShaderProgram;
class Font;
class Atlas;

// Менеджер для управления ресурсами
class Assets {
	std::unordered_map<std::string, Texture*> textures;
	std::unordered_map<std::string, ShaderProgram*> shaders;
    std::unordered_map<std::string, Font*> fonts;
	std::unordered_map<std::string, Atlas*> atlases;
public:
	~Assets(); // Деструктор

    // Методы для работы с текстурами
	Texture* getTexture(std::string name) const; // Получает текстуру по имени
	bool store(Texture* texture, std::string name); // Сохраняет текстуру в менеджере ресурсов.

    // Методы для работы с шейдерами
	ShaderProgram* getShader(std::string name) const; // Получает шейдер по имени
	bool store(ShaderProgram* shader, std::string name); // Сохраняет шейдер в менеджере ресурсов.

    // Методы для работы с шрифтами
	Font* getFont(std::string name) const; // Получает шрифт по имени
	bool store(Font* font, std::string name); // Сохраняет шрифт в менеджере ресурсов.

	Atlas* getAtlas(std::string name) const;
	bool store(Atlas* atlas, std::string name);
};

#endif // ASSETS_ASSETS_H_
