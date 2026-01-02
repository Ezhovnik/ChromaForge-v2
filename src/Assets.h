#ifndef SRC_ASSETS_H_
#define SRC_ASSETS_H_

#include <string>
#include <unordered_map>

class Texture;
class ShaderProgram;

// Менеджер для управления ресурсами
class Assets {
	std::unordered_map<std::string, Texture*> textures;
	std::unordered_map<std::string, ShaderProgram*> shaders;
public:
	~Assets(); // Конструктор

    // Методы для работы с текстурами
	Texture* getTexture(std::string name); // Получает текстуру по имени
	bool store(Texture* texture, std::string name); // Сохраняет текстуру в менеджере ресурсов.

    // Методы для работы с шейдерами
	ShaderProgram* getShader(std::string name); // Получает шейдер по имени
	bool store(ShaderProgram* shader, std::string name); // Сохраняет шейдер в менеджере ресурсов.
};

#endif // SRC_ASSETS_H_
