#include "asset_loaders.h"

#include <filesystem>
#include <stdexcept>

#include "Assets.h"
#include "AssetsLoader.h"
#include "../coders/imageio.h"
#include "../files/files.h"
#include "../graphics/core/ShaderProgram.h"
#include "../graphics/core/Texture.h"
#include "../graphics/core/ImageData.h"
#include "../graphics/core/Atlas.h"
#include "../graphics/core/Font.h"
#include "../debug/Logger.h"
#include "../files/engine_paths.h"
#include "../coders/json.h"
#include "../graphics/core/TextureAnimation.h"
#include "../frontend/UIDocument.h"
#include "../audio/audio.h"
#include "../coders/GLSLExtension.h"

static bool animation(
    Assets* assets, 
	const ResPaths* paths,
    const std::string& atlasName, 
    const std::string& directory, 
    const std::string& name, 
	Atlas* dstAtlas
);

asset_loader::postfunc asset_loader::shader(
	AssetsLoader*,
	const ResPaths* paths, 
	const std::string filename, 
	const std::string name, 
	std::shared_ptr<AssetsConfig>)
{
	// Формируем пути к файлам вершинного и фрагментного шейдеров
    std::filesystem::path vertexFile = paths->find(filename + ".vert");
    std::filesystem::path fragmentFile = paths->find(filename + ".frag");

	// Читаем исходный код шейдеров из файлов
    std::string vertexSource = files::read_string(vertexFile);
    std::string fragmentSource = files::read_string(fragmentFile);

	vertexSource = ShaderProgram::preprocessor->process(vertexFile, vertexSource);
    fragmentSource = ShaderProgram::preprocessor->process(fragmentFile, fragmentSource);

	// Сохраняем шейдер в менеджере ресурсов под указанным именем
	return [=](auto assets) {
        assets->store(ShaderProgram::create(
            vertexFile.u8string(),
            fragmentFile.u8string(),
            vertexSource, 
			fragmentSource
        ), name);
    };
}

asset_loader::postfunc asset_loader::texture(
	AssetsLoader*,
	const ResPaths* paths, 
	const std::string filename, 
	const std::string name, 
	std::shared_ptr<AssetsConfig>)
{
	std::shared_ptr<ImageData> image(imageio::read(paths->find(filename + ".png").u8string()).release());
	return [name, image](auto assets) {
        assets->store(Texture::from(image.get()), name);
    };
}

asset_loader::postfunc asset_loader::font(
	AssetsLoader*,
	const ResPaths* paths, 
	const std::string filename, 
	const std::string name, 
	std::shared_ptr<AssetsConfig>)
{
    auto pages = std::make_shared<std::vector<std::unique_ptr<ImageData>>>();
	for (size_t i = 0; i <= 4; ++i) {
		// Формируем имя файла страницы: filename_i.png
		std::string page_name = filename + "_" + std::to_string(i) + ".png"; 
        page_name = paths->find(page_name).string(); // ищем полный путь
		pages->push_back(imageio::read(page_name));
	}

	return [=](auto assets) {
        int res = pages->at(0)->getHeight() / 16;
        std::vector<std::unique_ptr<Texture>> textures;
        for (auto& page : *pages) {
            textures.emplace_back(Texture::from(page.get()));
        }
        assets->store(new Font(std::move(textures), res, 4), name);
    };
}

/**
 * Вспомогательная функция для добавления одного изображения в строитель атласа.
 * Проверяет расширение .png, уникальность имени и загружает изображение.
 * При необходимости конвертирует в формат RGBA и исправляет альфа-канал.
 */
static bool append_atlas(AtlasBuilder& atlas, const std::filesystem::path& file) {
	std::string name = file.stem().string();
	if (atlas.has(name)) return false;

	// Загружаем изображение
	auto image = imageio::read(file.string());
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
	AssetsLoader*,
	const ResPaths* paths, 
	const std::string directory, 
	const std::string name, 
	std::shared_ptr<AssetsConfig>)
{
	AtlasBuilder builder;

	// Проходим по всем файлам в указанной директории и пытаемся добавить каждый PNG в атлас
	for (auto const& file : paths->listdir(directory)) {
		if (!imageio::is_read_supported(file.extension().u8string())) continue;
		if (!append_atlas(builder, file)) continue;
	}

	std::set<std::string> names = builder.getNames();
	Atlas* atlas = builder.build(2, false).release();
	return [=](auto assets) {
        atlas->prepare();
        assets->store(atlas, name);

        for (const auto& file : names) {
            animation(assets, paths, name, directory, file, atlas);
        }
    };
}

static void read_anim_file(
    const std::string& animFile,
    std::vector<std::pair<std::string, int>>& frameList
) {
    auto root = files::read_json(animFile);
    auto frameArr = root->list("frames");
    float frameDuration = DEFAULT_FRAME_DURATION;
    std::string frameName;

    if (frameArr) {
        for (size_t i = 0; i < frameArr->size(); i++) {
            auto currentFrame = frameArr->list(i);

            frameName = currentFrame->str(0);
            if (currentFrame->size() > 1) {
                frameDuration = currentFrame->integer(1);
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

    frame.dstPos = glm::ivec2(region.u1 * dstWidth, region.v1 * dstHeight);
    frame.size = glm::ivec2(region.u2 * dstWidth, region.v2 * dstHeight) - frame.dstPos;

    for (const auto& elem : frameList) {
        if (!srcAtlas->has(elem.first)) {
			LOG_ERROR("Unknown frame name: {}", elem.first);
            continue;
        }
        region = srcAtlas->get(elem.first);
        if (elem.second > 0) {
            frame.duration = static_cast<float>(elem.second) / 1000.0f;
        }
        frame.srcPos = glm::ivec2(region.u1 * srcWidth, srcHeight - region.v2 * srcHeight);
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

static bool animation(
	Assets* assets, 
	const ResPaths* paths,
    const std::string& atlasName, 
    const std::string& directory, 
    const std::string& name, 
	Atlas* dstAtlas)
{
	// Формируем пути к папкам с анимациями и блоками
	std::string animsDir = directory + "/animation";

	// Ищем подпапку в animsDir, имя которой совпадает с текстурой
	for (const auto& folder : paths->listdir(animsDir)) {
		if (!std::filesystem::is_directory(folder)) continue;
		if (folder.filename().u8string() != name) continue;
		if (std::filesystem::is_empty(folder)) continue;

		AtlasBuilder builder;
		// Добавляем базовую текстуру из blocksDir (статичный кадр)
		append_atlas(builder, paths->find(directory + "/" + name + ".png"));

		// Если есть файл описания анимации, читаем его
		std::vector<std::pair<std::string, int>> frameList;
        std::string animFile = folder.u8string() + "/animation.json";
		if (std::filesystem::exists(animFile)) read_anim_file(animFile, frameList);

		// Проходим по файлам в папке анимации
		for (const auto& file : paths->listdir(animsDir + "/" + name)) {
			// Если есть список кадров из JSON, добавляем только те, что в списке
			if (!frameList.empty() && !contains(frameList, file.stem().u8string())) {
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
        assets->store(srcAtlas.release(), atlasName + "/" + name + "_animation");
		assets->store(animation);

		return true;
	}

	// Если папка с анимацией не найдена, всё равно считаем успехом (просто нет анимации)
	return true;
}

asset_loader::postfunc asset_loader::layout(
	AssetsLoader*,
	const ResPaths* paths, 
	const std::string file, 
	const std::string name, 
	std::shared_ptr<AssetsConfig> config)
{
    return [=](auto assets) {
        try {
            auto cfg = std::dynamic_pointer_cast<LayoutConfig>(config);
            auto document = UIDocument::read(cfg->env, name, file);
            assets->store(document.release(), name);
        } catch (const parsing_error& err) {
			LOG_ERROR("Failed to parse layout XML '{}'. What: {}", file, err.errorLog());
            throw std::runtime_error("failed to parse layout XML '" + file + "':\n" + err.errorLog());
        }
    };
}

asset_loader::postfunc asset_loader::sound(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string file,
    const std::string name,
    std::shared_ptr<AssetsConfig> config)
{
    auto cfg = std::dynamic_pointer_cast<SoundConfig>(config);
    bool keepPCM = cfg ? cfg->keepPCM : false;

    std::string extension = ".ogg";
    std::unique_ptr<audio::Sound> baseSound = nullptr;

    auto soundFile = paths->find(file + extension);
    if (std::filesystem::exists(soundFile)) {
        baseSound.reset(audio::load_sound(soundFile, keepPCM));
    }

    auto variantFile = paths->find(file + "_0" + extension);
    if (std::filesystem::exists(variantFile)) {
        baseSound.reset(audio::load_sound(variantFile, keepPCM));
    }

    for (uint i = 1; ; ++i) {
        auto variantFile = paths->find(file + "_" + std::to_string(i) + extension);
        if (!std::filesystem::exists(variantFile)) break;
        baseSound->variants.emplace_back(audio::load_sound(variantFile, keepPCM));
    }

    auto sound = baseSound.release();
    return [=](auto assets) {
        assets->store(sound, name);
    };
}
