#include <assets/asset_loaders.h>

#include <filesystem>
#include <stdexcept>

#include <assets/Assets.h>
#include <assets/AssetsLoader.h>
#include <coders/imageio.h>
#include <io/io.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <graphics/core/ImageData.h>
#include <graphics/core/Atlas.h>
#include <graphics/core/Font.h>
#include <debug/Logger.h>
#include <io/engine_paths.h>
#include <coders/json.h>
#include <graphics/core/TextureAnimation.h>
#include <frontend/UIDocument.h>
#include <audio/audio.h>
#include <coders/GLSLExtension.h>
#include <coders/commons.h>
#include <constants.h>
#include <graphics/commons/Model.h>
#include <coders/obj.h>
#include <objects/rigging.h>
#include <coders/vec3.h>
#include <util/stringutil.h>
#include <coders/cfmodel.h>

static bool load_animation(
    Assets* assets, 
	const ResPaths& paths,
    const std::string& atlasName, 
    const std::string& directory, 
    const std::string& name, 
	Atlas* dstAtlas
);

static auto process_program(const ResPaths& paths, const std::string& filename) {
    // Формируем пути к файлам вершинного и фрагментного шейдеров
    io::path vertexFile = paths.find(filename + ".vert");
    io::path fragmentFile = paths.find(filename + ".frag");

    // Читаем исходный код шейдеров из файлов
    std::string vertexSource = io::read_string(vertexFile);
    std::string fragmentSource = io::read_string(fragmentFile);

    auto& preprocessor = *ShaderProgram::preprocessor;

    auto vertex = preprocessor.process(vertexFile, vertexSource);
    auto fragment = preprocessor.process(fragmentFile, fragmentSource);
    return std::make_pair(vertex, fragment);
}

asset_loader::postfunc asset_loader::shader(
	AssetsLoader*,
	const ResPaths& paths, 
	const std::string& filename, 
	const std::string& name, 
	const std::shared_ptr<AssetsConfig>&)
{
	auto [vertex, fragment] = process_program(paths, filename);

    io::path vertexFile = paths.find(filename + ".vert");
    io::path fragmentFile = paths.find(filename + ".frag");

	std::string vertexSource = std::move(vertex.code);
    std::string fragmentSource = std::move(fragment.code);

	// Сохраняем шейдер в менеджере ресурсов под указанным именем
	return [=](auto assets) {
        assets->store(ShaderProgram::create(
            vertexFile.string(),
            fragmentFile.string(),
            vertexSource, 
			fragmentSource
        ), name);
    };
}

asset_loader::postfunc asset_loader::texture(
	AssetsLoader*,
	const ResPaths& paths, 
	const std::string& filename, 
	const std::string& name, 
	const std::shared_ptr<AssetsConfig>&)
{
    auto actualFile = paths.find(filename + ".png");
    try {
        std::shared_ptr<ImageData> image(imageio::read(actualFile));
        return [name, image, actualFile](auto assets) {
            assets->store(Texture::from(image.get()), name);
        };
    } catch (const std::runtime_error& err) {
        LOG_ERROR("{}: {}", actualFile.string(), err.what());
        return [](auto) {};
    }
}

asset_loader::postfunc asset_loader::font(
	AssetsLoader*,
	const ResPaths& paths, 
	const std::string& filename, 
	const std::string& name, 
	const std::shared_ptr<AssetsConfig>&)
{
    auto pages = std::make_shared<std::vector<std::unique_ptr<ImageData>>>();
	for (size_t i = 0; i <= 1024; ++i) {
		// Формируем имя файла страницы: filename_i.png
		std::string page_name = filename + "_" + std::to_string(i) + ".png"; 
        auto file = paths.find(page_name);
        if (io::exists(file)) {
            pages->push_back(imageio::read(file));
        } else if (i == 0) {
            LOG_ERROR("Font must have page 0");
            throw std::runtime_error("Font must have page 0");
        } else {
            pages->push_back(nullptr);
        }
	}

	return [=](auto assets) {
        int res = pages->at(0)->getHeight() / 16;
        std::vector<std::unique_ptr<Texture>> textures;
        for (auto& page : *pages) {
            if (page == nullptr) {
                textures.emplace_back(nullptr);
            } else {
                auto texture = Texture::from(page.get());
                texture->setMipMapping(false, true);
                textures.emplace_back(std::move(texture));
            }
        }
        assets->store(std::make_unique<Font>(std::move(textures), res, 4), name);
    };
}

asset_loader::postfunc asset_loader::posteffect(
    AssetsLoader*,
    const ResPaths& paths,
    const std::string& file,
    const std::string& name,
    const std::shared_ptr<AssetsConfig>& settings
) {
    io::path effectFile = paths.find(file + ".glsl");
    std::string effectSource = io::read_string(effectFile);

    auto& preprocessor = *ShaderProgram::preprocessor;
    preprocessor.addHeader(
        "__effect__", preprocessor.process(effectFile, effectSource, true)
    );

    auto [vertex, fragment] = process_program(paths, SHADERS_FOLDER + "/effect");
    auto params = std::move(fragment.params);

    std::string vertexSource = std::move(vertex.code);
    std::string fragmentSource = std::move(fragment.code);

    return [=](auto assets) {
        auto program = ShaderProgram::create(
            effectFile.string(),
            effectFile.string(),
            vertexSource,
            fragmentSource
        );
        assets->store(
            std::make_shared<PostEffect>(std::move(program), params), name
        );
    };
}

/**
 * Вспомогательная функция для добавления одного изображения в строитель атласа.
 * Проверяет расширение .png, уникальность имени и загружает изображение.
 * При необходимости конвертирует в формат RGBA и исправляет альфа-канал.
 */
static bool append_atlas(AtlasBuilder& atlas, const io::path& file) {
	std::string name = file.stem();
	if (atlas.has(name)) return false;

	// Загружаем изображение
	auto image = imageio::read(file);
	if (image == nullptr) {
		LOG_ERROR("Failed to load atlas entry '{}'", name);
		return false;
	}

	// Если формат не RGBA, конвертируем
	if (image->getFormat() != ImageFormat::rgba8888) image.reset(toRGBA(image.get())); 

	// Исправляем "чёрный" альфа-канал
	image->fixAlphaColor(); 

	// Добавляем изображение в строитель атласа
	atlas.add(name, std::move(image));

	return true;
}

asset_loader::postfunc asset_loader::atlas(
	AssetsLoader* loader,
	const ResPaths& paths, 
	const std::string& directory, 
	const std::string& name, 
	const std::shared_ptr<AssetsConfig>& config
) {
    auto atlasConfig = std::dynamic_pointer_cast<AtlasConfig>(config);
    if (atlasConfig && atlasConfig->type == AtlasType::Separate) {
        for (const auto& file : paths.listdir(directory)) {
            if (!imageio::is_read_supported(file.extension())) continue;
            loader->add(
                AssetType::Texture,
                directory + "/" + file.stem(),
                name + "/" + file.stem()
            );
        }
        return [](auto){};
    }

	AtlasBuilder builder;

	// Проходим по всем файлам в указанной директории и пытаемся добавить каждый PNG в атлас
	for (auto const& file : paths.listdir(directory)) {
		if (!imageio::is_read_supported(file.extension())) continue;
		if (!append_atlas(builder, file)) continue;
	}

	std::set<std::string> names = builder.getNames();
	Atlas* atlas = builder.build(2, false).release();
	return [=](auto assets) {
        atlas->prepare();
        assets->store(std::unique_ptr<Atlas>(atlas), name);

        for (const auto& file : names) {
            load_animation(assets, paths, name, directory, file, atlas);
        }
    };
}

static void read_anim_file(
    const std::string& animFile,
    std::vector<std::pair<std::string, int>>& frameList
) {
    auto root = io::read_json(animFile);
    float frameDuration = DEFAULT_FRAME_DURATION;
    std::string frameName;

    if (auto found = root.at("frames")) {
        auto& frameArr = *found;
        for (size_t i = 0; i < frameArr.size(); ++i) {
            const auto& currentFrame = frameArr[i];

            frameName = currentFrame[0].asString();
            if (currentFrame.size() > 1) {
                frameDuration = currentFrame[1].asNumber();
            }
            frameList.emplace_back(frameName, frameDuration);
        }
    }
}

static TextureAnimation create_animation(
    Atlas* srcAtlas,
    Atlas* dstAtlas,
    const std::string& name,
    const std::set<std::string>& frameNames,
    const std::vector<std::pair<std::string, int>>& frameList
) {
    Texture* srcTex = srcAtlas->getTexture();
    Texture* dstTex = dstAtlas->getTexture();
    UVRegion region = dstAtlas->get(name);

    TextureAnimation animation(srcTex, dstTex);
    Frame frame;

    uint dstWidth = dstTex->getWidth();
    uint dstHeight = dstTex->getHeight();

    uint srcWidth = srcTex->getWidth();
    uint srcHeight = srcTex->getHeight();

    const int extension = 2;
    frame.dstPos = glm::ivec2(region.u1 * dstWidth, region.v1 * dstHeight) - extension;
    frame.size = glm::ivec2(region.u2 * dstWidth, region.v2 * dstHeight) - frame.dstPos + extension;

    for (const auto& elem : frameList) {
        if (!srcAtlas->has(elem.first)) {
			LOG_ERROR("Unknown frame name: {}", elem.first);
            continue;
        }
        region = srcAtlas->get(elem.first);
        if (elem.second > 0) {
            frame.duration = static_cast<float>(elem.second) / 1000.0f;
        }
        frame.srcPos = glm::ivec2(region.u1 * srcWidth, srcHeight - region.v2 * srcHeight) - extension;
        animation.addFrame(frame);
    }
    return animation;
}

inline bool contains(
    const std::vector<std::pair<std::string, int>>& frameList,
    const std::string& frameName
) {
    for (const auto& elem : frameList) {
        if (frameName == elem.first) return true;
    }
    return false;
}

static bool load_animation(
	Assets* assets, 
	const ResPaths& paths,
    const std::string& atlasName, 
    const std::string& directory, 
    const std::string& name, 
	Atlas* dstAtlas)
{
	// Формируем пути к папкам с анимациями и блоками
	std::string animsDir = directory + "/animation";

	// Ищем подпапку в animsDir, имя которой совпадает с текстурой
	for (const auto& folder : paths.listdir(animsDir)) {
		if (!io::is_directory(folder)) continue;
		if (folder.name() != name) continue;
		// FIXME: if (std::filesystem::is_empty(folder)) continue;

		AtlasBuilder builder;
		// Добавляем базовую текстуру из blocksDir (статичный кадр)
		append_atlas(builder, paths.find(directory + "/" + name + ".png"));

		// Если есть файл описания анимации, читаем его
		std::vector<std::pair<std::string, int>> frameList;
        std::string animFile = folder.string() + "/animation.json";
		if (io::exists(animFile)) read_anim_file(animFile, frameList);

		// Проходим по файлам в папке анимации
		for (const auto& file : paths.listdir(animsDir + "/" + name)) {
			// Если есть список кадров из JSON, добавляем только те, что в списке
			if (!frameList.empty() && !contains(frameList, file.stem())) {
                continue;
			}
			// Добавляем изображение в строитель атласа анимации
			if (!append_atlas(builder, file)) continue;
		}

		// Строим исходный атлас анимации
		auto srcAtlas = builder.build(2, true);

		if (frameList.empty()) {
			// Если JSON с кадрами не задан, используем все имена из srcAtlas в порядке добавления
			for (const auto& frameName : builder.getNames()) {
                frameList.emplace_back(frameName, 0);
			}
		}

		auto animation = create_animation(
            srcAtlas.get(), dstAtlas, name, builder.getNames(), frameList
        );
        assets->store(std::move(srcAtlas), atlasName + "/" + name + "_animation");
		assets->store(animation);

		return true;
	}

	// Если папка с анимацией не найдена, всё равно считаем успехом (просто нет анимации)
	return true;
}

asset_loader::postfunc asset_loader::layout(
	AssetsLoader*,
	const ResPaths&, 
	const std::string& file, 
	const std::string& name, 
	const std::shared_ptr<AssetsConfig>& config)
{
    return [=](auto assets) {
        try {
            auto cfg = std::dynamic_pointer_cast<LayoutConfig>(config);
            size_t pos = name.find(':');
            auto prefix = name.substr(0, pos);
            assets->store(
                UIDocument::read(
                    *cfg->gui,
                    cfg->env,
                    name,
                    file,
                    prefix + ":layouts/" + name.substr(pos + 1) + ".xml"
                ),
                name
            );
        } catch (const parsing_error& err) {
			LOG_ERROR("Failed to parse layout XML '{}'. What: {}", file, err.errorLog());
            throw std::runtime_error("failed to parse layout XML '" + file + "':\n" + err.errorLog());
        }
    };
}

asset_loader::postfunc asset_loader::sound(
    AssetsLoader*,
    const ResPaths& paths,
    const std::string& file,
    const std::string& name,
    const std::shared_ptr<AssetsConfig>& config)
{
    auto cfg = std::dynamic_pointer_cast<SoundConfig>(config);
    bool keepPCM = cfg ? cfg->keepPCM : false;

    std::unique_ptr<audio::Sound> baseSound = nullptr;

    static std::vector<std::string> extensions {".ogg", ".wav"};
    std::string extension;
    for (size_t i = 0; i < extensions.size(); ++i) {
        extension = extensions[i];
        auto soundFile = paths.find(file + extension);
        if (io::exists(soundFile)) {
            baseSound = audio::load_sound(soundFile, keepPCM);
            break;
        }
        auto variantFile = paths.find(file + "_0" + extension);
        if (io::exists(variantFile)) {
            baseSound = audio::load_sound(variantFile, keepPCM);
            break;
        }
    }

    if (baseSound == nullptr) {
        LOG_ERROR("Could not to find sound: {}", file);
        throw std::runtime_error("Could not to find sound: " + file);
    }

    for (uint i = 1; ; ++i) {
        auto variantFile = paths.find(file + "_" + std::to_string(i) + extension);
        if (!io::exists(variantFile)) break;
        baseSound->variants.emplace_back(audio::load_sound(variantFile, keepPCM));
    }

    auto sound = baseSound.release();
    return [=](auto assets) {
        assets->store(std::unique_ptr<audio::Sound>(sound), name);
    };
}

static void request_textures(AssetsLoader* loader, const model::Model& model) {
    for (auto& mesh : model.meshes) {
        if (mesh.texture.find('$') == std::string::npos && mesh.texture.find(':') == std::string::npos) {
            auto filename = TEXTURES_FOLDER + "/" + mesh.texture;
            loader->add(
                AssetType::Texture, filename, mesh.texture, nullptr
            );
        }
    }
}

asset_loader::postfunc asset_loader::model(
    AssetsLoader* loader,
    const ResPaths& paths,
    const std::string& file,
    const std::string& name,
    const std::shared_ptr<AssetsConfig>&
) {
    auto path = paths.find(file + ".vec3");
    if (io::exists(path)) {
        auto bytes = io::read_bytes_buffer(path);
        auto modelVEC3 = std::make_shared<vec3::File>(vec3::load(path.string(), bytes));
        return [loader, name, modelVEC3=std::move(modelVEC3)](Assets* assets) {
            for (auto& [modelName, model] : modelVEC3->models) {
                request_textures(loader, model.model);
                std::string fullName = name;
                if (name != modelName) {
                    fullName += "." + modelName;
                }
                assets->store(
                    std::make_unique<model::Model>(model.model),
                    fullName
                );
                LOG_INFO("Store model {} as {}", util::quote(modelName), util::quote(fullName));
            }
        };
    }
    path = paths.find(file + ".obj");
    auto text = io::read_string(path);
    if (io::exists(path)) {
        auto text = io::read_string(path);
        try {
            auto model = obj::parse(path.string(), text).release();
            return [=](Assets* assets) {
                request_textures(loader, *model);
                assets->store(std::unique_ptr<model::Model>(model), name);
            };
        } catch (const parsing_error& err) {
            LOG_ERROR("Failed to load model '{}': {}", file, err.errorLog());
            throw;
        }
    }
    path = paths.find(file + ".xml");
    if (io::exists(path)) {
        auto text = io::read_string(path);
        try {
            auto model = cfmodel::parse(path.string(), text).release();
            return [=](Assets* assets) {
                request_textures(loader, *model);
                assets->store(std::unique_ptr<model::Model>(model), name);
            };
        } catch (const parsing_error& err) {
            LOG_ERROR("Failed to load model '{}': {}", file, err.errorLog());
            throw;
        }
    }
    LOG_ERROR("Could not to find model {}", util::quote(file));
    throw std::runtime_error("Could not to find model " + util::quote(file));
}
