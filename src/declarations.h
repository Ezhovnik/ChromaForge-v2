#ifndef SRC_DECLARATIONS_H_
#define SRC_DECLARATIONS_H_

#include <iostream>
#include "Assets.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "window/Window.h"

#include "voxels/Block.h"

// Загружает и регистрирует шейдерную программу в менеджере ресурсов.
bool _load_shader(Assets* assets, std::string vertex_file, std::string fragment_file, std::string name){
	ShaderProgram* shader = loadShaderProgram(vertex_file, fragment_file);
	if (shader == nullptr){
		std::cerr << "ERROR::Failed to load shader '" << name << "'" << std::endl;
		return false;
	}

	return assets->store(shader, name);
}

// Загружает и регистрирует текстуру в менеджере ресурсов.
bool _load_texture(Assets* assets, std::string filename, std::string name){
	Texture* texture = loadTexture(filename);
	if (texture == nullptr){
		std::cerr << "ERROR::Failed to load texture '" << name << "'" << std::endl;
		return false;
	}

	return assets->store(texture, name);
}

// Инициализирует все графические ресурсы (шейдеры и текстуры).
bool initialize_assets(Assets* assets) {
    return _load_shader(assets, "../res/shaders/default.vert", "../res/shaders/default.frag", "default")
        && _load_shader(assets, "../res/shaders/crosshair.vert", "../res/shaders/crosshair.frag", "crosshair")
        && _load_shader(assets, "../res/shaders/lines.vert", "../res/shaders/lines.frag", "lines")
        && _load_texture(assets, "../res/textures/atlas.png", "blocks");;
}

// Инициализирует все типы блоков
void setup_definitions() {
    // Воздух
    Block* block = new Block(0, 0);
    block->drawGroup = 1;
    block->lightPassing = true;
    block->obstacle = false;
    block->selectable = false;
    Block::blocks[block->id] = block;

    // Мох
    block = new Block(1, 1);
    Block::blocks[block->id] = block;

    // Земля
    block = new Block(2, 2);
    Block::blocks[block->id] = block;

    // Светокамень
    block = new Block(3, 3);
    block->emission[0] = 14;
    block->emission[1] = 12;
    block->emission[2] = 3;
    Block::blocks[block->id] = block;

    // Стекло
    block = new Block(4, 4);
    block->drawGroup = 2;
    block->lightPassing = true;
    Block::blocks[block->id] = block;

    // Доски
    block = new Block(5, 5);
    Block::blocks[block->id] = block;

    // Бревно
    block = new Block(6, 6);
    block->textureFaces[2] = 7;
    block->textureFaces[3] = 7;
    Block::blocks[block->id] = block;

    // Листва
    block = new Block(7, 8);
    block->drawGroup = 3;
    block->lightPassing = true;
    Block::blocks[block->id] = block;

    // Вода
    block = new Block(8, 9);
	block->drawGroup = 4;
	block->lightPassing = true;
	block->skyLightPassing = false;
	block->obstacle = false;
	block->selectable = false;
	Block::blocks[block->id] = block;
}

#endif // SRC_DECLARATIONS_H_
