#include "command_line.h"

#include <filesystem>

bool parse_cmdline(int argc, char** argv, EnginePaths& paths) {
	ArgsReader reader(argc, argv);
	reader.skip(); // пропускаем имя исполняемого файла (argv[0])
	while (reader.hasNext()) {
		std::string token = reader.next();

		// Если аргумент начинается с '-', это ключ
		if (reader.isKeywordArg()) {
			if (token == "--res") {
				// Читаем следующий аргумент как путь к папке ресурсов
				token = reader.next();
				if (!std::filesystem::is_directory(std::filesystem::path(token))) {
					std::cerr << token << " is not a directory" << std::endl;
					throw std::runtime_error(token + " is not a directory");
				}
				paths.setResources(std::filesystem::path(token));
				std::cout << "Resources folder: " << token << std::endl;
			} else if (token == "--dir") {
				// Читаем следующий аргумент как путь к папке пользовательских файлов
				token = reader.next();
				if (!std::filesystem::is_directory(std::filesystem::path(token))) {
					std::filesystem::create_directories(std::filesystem::path(token));
				}
				paths.setUserfiles(std::filesystem::path(token));
				std::cout << "Userfiles folder: " << token << std::endl;
			} else if (token == "--help" || token == "-h") {
				// Выводим справку и сообщаем, что запуск нужно прервать.
				std::cout << "ChromaForge command-line arguments:" << std::endl;
				std::cout << " --res [path] - set resources directory" << std::endl;
				std::cout << " --dir [path] - set userfiles directory" << std::endl;
				return false;
			} else {
				// Неизвестный ключ
				std::cerr << "Unknown argument " << token << std::endl;
			}
		} else {
			// Если аргумент не является ключом, но ожидался только ключ
			std::cerr << "Unexpected token" << std::endl;
		}
	}
	// Все аргументы успешно обработаны, продолжаем выполнение
	return true;
}
