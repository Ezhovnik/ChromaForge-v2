#include "Assets.h"

#include "../graphics/core/Texture.h"
#include "../graphics/core/ShaderProgram.h"
#include "../graphics/core/Font.h"
#include "../graphics/core/Atlas.h"
#include "../logger/Logger.h"
#include "../frontend/UIDocument.h"
#include "../logic/scripting/scripting.h"
#include "../audio/audio.h"

Assets::~Assets() {
}

Texture* Assets::getTexture(std::string name) const {
    auto it = textures.find(name);
    if (it != textures.end()) return it->second.get();
	return nullptr;
}

void Assets::store(Texture* texture, std::string name){
    textures.emplace(name, texture);
}

ShaderProgram* Assets::getShader(std::string name) const{
    auto it = shaders.find(name);
    if (it != shaders.end()) return it->second.get();
    return nullptr;
}

void Assets::store(ShaderProgram* shader, std::string name){
    shaders.emplace(name, shader);
}

Font* Assets::getFont(std::string name) const{
    auto it = fonts.find(name);
    if (it != fonts.end()) return it->second.get();
    return nullptr;
}

void Assets::store(Font* font, std::string name){
    fonts.emplace(name, font);
}

Atlas* Assets::getAtlas(std::string name) const {
    auto it = atlases.find(name);
    if (it != atlases.end()) return it->second.get();
    return nullptr;
}

void Assets::store(Atlas* atlas, std::string name){
    atlases.emplace(name, atlas);
}

const std::vector<TextureAnimation>& Assets::getAnimations() {
	return animations;
}

void Assets::store(const TextureAnimation& animation) {
	animations.emplace_back(animation);
}

UIDocument* Assets::getLayout(std::string name) const {
    auto it = layouts.find(name);
    if (it != layouts.end()) return it->second.get();
    return nullptr;
}

void Assets::store(UIDocument* layout, std::string name){
    layouts[name] = std::shared_ptr<UIDocument>(layout);
}

audio::Sound* Assets::getSound(std::string name) const {
	auto found = sounds.find(name);
	if (found == sounds.end()) return nullptr;
	return found->second.get();
}

void Assets::store(audio::Sound* sound, std::string name) {
	sounds.emplace(name, sound);
}

void Assets::extend(const Assets& assets) {
    // Копируем текстуры
    for (auto entry : assets.textures) {
        textures[entry.first] = entry.second;
    }

    // Копируем шейдеры
    for (auto entry : assets.shaders) {
        shaders[entry.first] = entry.second;
    }

    // Копируем шрифты
    for (auto entry : assets.fonts) {
        fonts[entry.first] = entry.second;
    }

    // Копируем атласы
    for (auto entry : assets.atlases) {
        atlases[entry.first] = entry.second;
    }

    // Копируем макеты
    for (auto entry : assets.layouts) {
		layouts[entry.first] = entry.second;
	}

    // Копируем звуки
    for (auto entry : assets.sounds) {
		sounds[entry.first] = entry.second;
	}

    // Перезаписываем анимации
    animations.clear();
    for (auto entry : assets.animations) {
		animations.emplace_back(entry);
	}
}
