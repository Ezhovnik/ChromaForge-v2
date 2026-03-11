#include "asset_loaders.h"

#include <filesystem>

#include "Assets.h"
#include "AssetsLoader.h"
#include "../coders/png.h"
#include "../files/files.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../graphics/ImageData.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"
#include "../logger/Logger.h"
#include "../files/engine_paths.h"
#include "../coders/json.h"
#include "../graphics/TextureAnimation.h"
#include "../frontend/UIDocument.h"
#include "../logic/scripting/scripting.h"

bool asset_loader::shader(
	AssetsLoader&,
	Assets* assets, 
	const ResPaths* paths, 
	const std::string filename, 
	const std::string name, 
	std::shared_ptr<void>)
{
	// Формируем пути к файлам вершинного и фрагментного шейдеров
    std::filesystem::path vertexFile = paths->find(filename + ".vert");
    std::filesystem::path fragmentFile = paths->find(filename + ".frag");

	// Читаем исходный код шейдеров из файлов
    std::string vertexSource = files::read_string(vertexFile);
    std::string fragmentSource = files::read_string(fragmentFile);

	// Компилируем и линкуем шейдерную программу
	ShaderProgram* shader = ShaderProgram::create(
		vertexFile.string(),
		fragmentFile.string(),
		vertexSource,
		fragmentSource
	);

	if (shader == nullptr) {
        LOG_CRITICAL("Failed to load shader '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	// Сохраняем шейдер в менеджере ресурсов под указанным именем
	assets->store(shader, name);
	return true;
}

bool asset_loader::texture(
	AssetsLoader&, 
	Assets* assets, 
	const ResPaths* paths, 
	const std::string filename, 
	const std::string name, 
	std::shared_ptr<void>)
{
	// Загружаем PNG-изображение как текстуру (путь ищется через ResPaths)
	std::unique_ptr<Texture> texture(png::loadTexture(paths->find(filename).u8string()));
	if (texture == nullptr){
		LOG_CRITICAL("Failed to load texture '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	// Передаём владение текстурой в менеджер ресурсов
	assets->store(texture.release(), name);
	return true;
}

bool asset_loader::font(
	AssetsLoader&,
	Assets* assets, 
	const ResPaths* paths, 
	const std::string filename, 
	const std::string name, 
	std::shared_ptr<void>)
{
	// Вектор для хранения страниц шрифта (обычно 5 страниц: 0..4)
    std::vector<std::unique_ptr<Texture>> pages;
	for (size_t i = 0; i <= 4; ++i) {
		// Формируем имя файла страницы: filename_i.png
		std::string page_name = filename + "_" + std::to_string(i) + ".png"; 
        page_name = paths->find(page_name).string(); // ищем полный путь
		std::unique_ptr<Texture> texture(png::loadTexture(page_name));
		if (texture == nullptr){
            LOG_CRITICAL("Failed to load bitmap font '{}' (missing page {})", page_name, std::to_string(i));
            Logger::getInstance().flush();
			return false;
		}
		pages.push_back(std::move(texture));
	}

	int res = pages[0]->height / 16; // Размер одного символа

	// Создаём объект шрифта и сохраняем в менеджере
	assets->store(new Font(std::move(pages), res, 4), name);
	return true;
}

/**
 * Вспомогательная функция для добавления одного изображения в строитель атласа.
 * Проверяет расширение .png, уникальность имени и загружает изображение.
 * При необходимости конвертирует в формат RGBA и исправляет альфа-канал.
 */
static bool appendAtlas(AtlasBuilder& atlas, const std::filesystem::path& file) {
	if (file.extension() != ".png") return false;

	std::string name = file.stem().string();
	if (atlas.has(name)) return false;

	// Загружаем изображение
	std::shared_ptr<ImageData> image(png::loadImage(file.string()));
	if (image == nullptr) {
		LOG_ERROR("Failed to load atlas entry '{}'", name);
		Logger::getInstance().flush();
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

bool asset_loader::atlas(
	AssetsLoader&,
	Assets* assets, 
	const ResPaths* paths, 
	const std::string directory, 
	const std::string name, 
	std::shared_ptr<void>)
{
	AtlasBuilder builder;

	// Проходим по всем файлам в указанной директории и пытаемся добавить каждый PNG в атлас
	for (auto const& file : paths->listdir(directory)) {
		if (!appendAtlas(builder, file)) continue;
	}

	// Строим атлас с отступами в 2 пикселя
	Atlas* atlas = builder.build(2);
	assets->store(atlas, name);

	// Для каждой текстуры в атласе пытаемся загрузить соответствующую анимацию
	for (const auto& file : builder.getNames()) {
		asset_loader::animation(assets, paths, "textures", file, atlas);
	}

	return true;
}

bool asset_loader::animation(
	Assets* assets, 
	const ResPaths* paths, 
	const std::string directory, 
	const std::string name, 
	Atlas* dstAtlas)
{
	// Формируем пути к папкам с анимациями и блоками
	std::string animsDir = directory + "/animations";
	std::string blocksDir = directory + "/blocks";

	// Ищем подпапку в animsDir, имя которой совпадает с текстурой
	for (const auto& folder : paths->listdir(animsDir)) {
		if (!std::filesystem::is_directory(folder)) continue;
		if (folder.filename().string() != name) continue;
		if (std::filesystem::is_empty(folder)) continue;

		AtlasBuilder builder;
		// Добавляем базовую текстуру из blocksDir (статичный кадр)
		appendAtlas(builder, paths->find(blocksDir + "/" + name + ".png"));

		std::string animFile = folder.string() + "/animation.json";

		std::vector<std::pair<std::string, float>> frameList;

		// Если есть файл описания анимации, читаем его
		if (std::filesystem::exists(animFile)) {
			auto root = files::read_json(animFile);

			auto frameArr = root->list("frames");

			float frameDuration = DEFAULT_FRAME_DURATION;
			std::string frameName;

			if (frameArr) {
				for (size_t i = 0; i < frameArr->size(); ++i) {
					auto currentFrame = frameArr->list(i);

					frameName = currentFrame->str(0);
					if (currentFrame->size() > 1) {
						frameDuration = static_cast<float>(currentFrame->integer(1)) / 1000;
					}

					frameList.emplace_back(frameName, frameDuration);
				}
			}
		}

		// Проходим по файлам в папке анимации
		for (const auto& file : paths->listdir(animsDir + "/" + name)) {
			// Если есть список кадров из JSON, добавляем только те, что в списке
			if (!frameList.empty()) {
				bool contains = false;
				for (const auto& elem : frameList) {
					if (file.stem() == elem.first) {
						contains = true;
						break;
					}
				}
				if (!contains) continue;
			}
			// Добавляем изображение в строитель атласа анимации
			if (!appendAtlas(builder, file)) continue;
		}

		// Строим исходный атлас анимации
		std::unique_ptr<Atlas> srcAtlas(builder.build(2));

		Texture* srcTex = srcAtlas->getTexture();
		Texture* dstTex = dstAtlas->getTexture();

		TextureAnimation animation(srcTex, dstTex);
		Frame frame;
		// Получаем регион целевой текстуры в общем атласе
		UVRegion region = dstAtlas->get(name);

		// Вычисляем позицию и размер в целевом атласе (в пикселях)
		frame.dstPos = glm::ivec2(region.u1 * dstTex->width, region.v1 * dstTex->height);
		frame.size = glm::ivec2(region.u2 * dstTex->width, region.v2 * dstTex->height) - frame.dstPos;

		if (frameList.empty()) {
			// Если JSON с кадрами не задан, используем все имена из srcAtlas в порядке добавления
			for (const auto& elem : builder.getNames()) {
				region = srcAtlas->get(elem);
				frame.srcPos = glm::ivec2(region.u1 * srcTex->width, srcTex->height - region.v2 * srcTex->height);
				animation.addFrame(frame);
			}
		} else {
			// Иначе добавляем кадры в порядке, заданном в JSON
			for (const auto& elem : frameList) {
				if (!srcAtlas->has(elem.first)) {
					LOG_ERROR("Unknown frame name: '{}'", elem.first);
					continue;
				}
				region = srcAtlas->get(elem.first);
				frame.duration = elem.second;
				frame.srcPos = glm::ivec2(region.u1 * srcTex->width, srcTex->height - region.v2 * srcTex->height);
				animation.addFrame(frame);
			}
		}

		// Сохраняем исходный атлас анимации и саму анимацию в менеджере ресурсов
		assets->store(srcAtlas.release(), name + "_animation");
		assets->store(animation);

		return true;
	}

	// Если папка с анимацией не найдена, всё равно считаем успехом (просто нет анимации)
	return true;
}

bool asset_loader::layout(
	AssetsLoader& loader,
	Assets* assets, 
	const ResPaths* paths, 
	const std::string file, 
	const std::string name, 
	std::shared_ptr<void> config)
{
    try {
        LayoutConfig* cfg = reinterpret_cast<LayoutConfig*>(config.get());
        auto document = UIDocument::read(loader, cfg->env, name, file); // Читаем документ интерфейса из XML
        assets->store(document.release(), name);
        return true;
    } catch (const parsing_error& err) {
		LOG_ERROR("Failed to parse layout XML '{}'. Reason: {}", file, err.errorLog());
        return false;
    }
}
