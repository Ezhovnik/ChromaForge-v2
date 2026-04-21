#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include <optional>
#include <stdexcept>

#include <graphics/core/TextureAnimation.h>

class Assets;

/**
 * @brief Типы ресурсов, которые могут быть загружены через AssetsLoader.
 */
enum class AssetType {
     Texture, ///< Текстура
	Shader, ///< Шейдерная программа
	Font, ///< Растровый шрифт
	Atlas, ///< Атлас текстур
	Layout, ///< Макет интерфейса (XML)
     Sound, ///< Звук
     Model
};

namespace asset_loader {
     using postfunc = std::function<void(Assets*)>;

     using setupfunc = std::function<void(const Assets*)>;

     template<class T>
     void assets_setup(const Assets*);

     class error : public std::runtime_error {
     private:
          AssetType type;
          std::string filename;
          std::string reason;
     public:
          error(
               AssetType type, std::string filename, std::string reason
          ) : std::runtime_error(filename + ": " + reason),
               type(type),
               filename(std::move(filename)),
               reason(std::move(reason)) {
          }

          AssetType getAssetType() const {
               return type;
          }

          const std::string& getFilename() const {
               return filename;
          }

          const std::string& getReason() const {
               return reason;
          }
     };
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
	std::vector<TextureAnimation> animations; ///< Анимации, связанные с текстурами.

     using assets_map = std::unordered_map<std::string, std::shared_ptr<void>>;
     std::unordered_map<std::type_index, assets_map> assets;
     std::vector<asset_loader::setupfunc> setupFuncs;
public:
     Assets() = default;
     Assets(const Assets&) = delete;
	~Assets();

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

     template<class T>
     void store(std::unique_ptr<T> asset, const std::string& name) {
          assets[typeid(T)][name].reset(asset.release());
     }

     template<class T>
     T* get(const std::string& name) const {
          const auto& mapIter = assets.find(typeid(T));
          if (mapIter == assets.end()) return nullptr;
          const auto& map = mapIter->second;
          const auto& found = map.find(name);
          if (found == map.end()) return nullptr;
          return static_cast<T*>(found->second.get());
     }

     template<class T>
     std::optional<const assets_map*> getMap() const {
          const auto& mapIter = assets.find(typeid(T));
          if (mapIter == assets.end()) {
               return std::nullopt;
          }
          return &mapIter->second;
     }

     void setup() {
          for (auto& setupFunc : setupFuncs) {
               setupFunc(this);
          }
     }

     void addSetupFunc(asset_loader::setupfunc setupfunc) {
          setupFuncs.push_back(setupfunc);
     }
};

template<class T>
void asset_loader::assets_setup(const Assets* assets) {
     if (auto mapPtr = assets->getMap<T>()) {
          for (const auto& entry : **mapPtr) {
               static_cast<T*>(entry.second.get())->setup();
          }
     }
}

#endif // ASSETS_ASSETS_H_
