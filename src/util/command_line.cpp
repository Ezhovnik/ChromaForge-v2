#include "command_line.h"

#include <filesystem>

bool perform_keyword(ArgsReader& reader, const std::string& keyword, EnginePaths& paths) {
    if (keyword == "--res") {
		// Читаем следующий аргумент как путь к папке ресурсов
		auto token = reader.next();
		if (!std::filesystem::is_directory(std::filesystem::path(token))) {
			std::cerr << token << " is not a directory" << std::endl;
			throw std::runtime_error(token + " is not a directory");
		}
		paths.setResources(std::filesystem::path(token));
		std::cout << "Resources folder: " << token << std::endl;
	} else if (keyword == "--dir") {
		// Читаем следующий аргумент как путь к папке пользовательских файлов
		auto token = reader.next();
		if (!std::filesystem::is_directory(std::filesystem::path(token))) {
			std::filesystem::create_directories(std::filesystem::path(token));
		}
		paths.setUserfiles(std::filesystem::path(token));
		std::cout << "Userfiles folder: " << token << std::endl;
	} else if (keyword == "--help" || keyword == "-h") {
		// Выводим справку и сообщаем, что запуск нужно прервать.
		std::cout << "ChromaForge command-line arguments:" << std::endl;
		std::cout << " --res [path] - set resources directory" << std::endl;
		std::cout << " --dir [path] - set userfiles directory" << std::endl;
		return false;
	} else {
		// Неизвестный ключ
		std::cerr << "Unknown argument " << keyword << std::endl;
		return false;
	}
    return true;
}

bool parse_cmdline(int argc, char** argv, EnginePaths& paths) {
	ArgsReader reader(argc, argv);
	reader.skip(); // пропускаем имя исполняемого файла (argv[0])
	while (reader.hasNext()) {
		std::string token = reader.next();

		if (reader.isKeywordArg()) {
			// Если аргумент начинается с '-', это ключ
			if (!perform_keyword(reader, token, paths)) return false;
		} else {
			// Если аргумент не является ключом, но ожидался только ключ
			std::cerr << "Unexpected token" << std::endl;
		}
	}
	// Все аргументы успешно обработаны, продолжаем выполнение
	return true;
}
