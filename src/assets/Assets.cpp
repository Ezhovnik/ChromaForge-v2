#include "Assets.h"

#include "../graphics/core/Texture.h"
#include "../graphics/core/ShaderProgram.h"
#include "../graphics/core/Font.h"
#include "../graphics/core/Atlas.h"
#include "../debug/Logger.h"
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

void Assets::store(std::unique_ptr<Texture> texture, std::string name){
    textures.emplace(name, std::move(texture));
}

ShaderProgram* Assets::getShader(std::string name) const{
    auto it = shaders.find(name);
    if (it != shaders.end()) return it->second.get();
    return nullptr;
}

void Assets::store(std::unique_ptr<ShaderProgram> shader, std::string name){
    shaders.emplace(name, std::move(shader));
}

Font* Assets::getFont(std::string name) const{
    auto it = fonts.find(name);
    if (it != fonts.end()) return it->second.get();
    return nullptr;
}

void Assets::store(std::unique_ptr<Font> font, std::string name){
    fonts.emplace(name, std::move(font));
}

Atlas* Assets::getAtlas(std::string name) const {
    auto it = atlases.find(name);
    if (it != atlases.end()) return it->second.get();
    return nullptr;
}

void Assets::store(std::unique_ptr<Atlas> atlas, std::string name){
    atlases.emplace(name, std::move(atlas));
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

void Assets::store(std::unique_ptr<UIDocument> layout, std::string name){
    layouts[name] = std::shared_ptr<UIDocument>(std::move(layout));
}

audio::Sound* Assets::getSound(std::string name) const {
	auto found = sounds.find(name);
	if (found == sounds.end()) return nullptr;
	return found->second.get();
}

void Assets::store(std::unique_ptr<audio::Sound> sound, std::string name) {
	sounds.emplace(name, std::move(sound));
}
