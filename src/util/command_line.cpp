#include <util/command_line.h>

#include <filesystem>
#include <iostream>

#include <files/engine_paths.h>
#include <engine.h>
#include <util/ArgsReader.h>

static bool perform_keyword(
	util::ArgsReader& reader, const std::string& keyword, CoreParameters& params
) {
    if (keyword == "--res") {
		// Читаем следующий аргумент как путь к папке ресурсов
		auto token = reader.next();
		params.resFolder = std::filesystem::u8path(token);
	} else if (keyword == "--dir") {
		// Читаем следующий аргумент как путь к папке пользовательских файлов
		auto token = reader.next();
		params.userFolder = std::filesystem::u8path(token);
	} else if (keyword == "--help" || keyword == "-h") {
		// Выводим справку и сообщаем, что запуск нужно прервать.
		std::cout << "ChromaForge command-line arguments:\n";
        std::cout << " --help - show help\n";
        std::cout << " --res <path> - set resources directory\n";
        std::cout << " --dir <path> - set userfiles directory\n";
        std::cout << " --headless - run in headless mode\n";
		std::cout << " --test <path> - test script file\n";
        std::cout << std::endl;
		return false;
	} else if (keyword == "--headless") {
        params.headless = true;
	} else if (keyword == "--test") {
		auto token = reader.next();
		params.testFile = std::filesystem::u8path(token);
	} else {
		// Неизвестный ключ
		throw std::runtime_error("Unknown argument " + keyword);
	}
    return true;
}

bool parse_cmdline(int argc, char** argv, CoreParameters& params) {
	util::ArgsReader reader(argc, argv);
	reader.skip(); // пропускаем имя исполняемого файла (argv[0])
	while (reader.hasNext()) {
		std::string token = reader.next();

		if (reader.isKeywordArg()) {
			// Если аргумент начинается с '-', это ключ
			if (!perform_keyword(reader, token, params)) return false;
		} else {
			// Если аргумент не является ключом, но ожидался только ключ
			std::cerr << "Unexpected token" << std::endl;
		}
	}
	// Все аргументы успешно обработаны, продолжаем выполнение
	return true;
}
