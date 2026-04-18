#include "command_line.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>

#include "files/engine_paths.h"

/**
 * @brief Утилита для последовательного чтения аргументов командной строки.
 *
 * Позволяет проходить по аргументам, проверять наличие следующего,
 * определять, является ли текущий аргумент ключом (начинается с '-'),
 * и получать значение аргумента.
 */
class ArgsReader {
private:
	int argc; ///< Количество аргументов
	char** argv; ///< Массив аргументов
	int pos = 0;
	const char* last = ""; ///< Последний прочитанный аргумент
public:
	/**
     * @brief Конструктор.
     * @param argc Количество аргументов (из main).
     * @param argv Массив аргументов (из main).
     */
	ArgsReader(int argc, char** argv) : argc(argc), argv(argv) {}

	/**
     * @brief Пропускает текущий аргумент.
     */
	void skip() {
		pos++;
	}

	/**
     * @brief Проверяет, есть ли ещё непрочитанные аргументы.
     * @return true, если позиция меньше argc, иначе false.
     */
	bool hasNext() const {
		return pos < argc && strlen(argv[pos]);
	}

	/**
     * @brief Проверяет, является ли последний прочитанный аргумент ключом (начинается с '-').
     * @return true, если первый символ last — '-', иначе false.
     */
	bool isKeywordArg() const {
		return last[0] == '-';
	}

	/**
     * @brief Возвращает следующий аргумент и перемещает позицию.
     * @return Строка с аргументом.
     * @throw std::runtime_error если аргументов больше нет.
     */
	std::string next() {
		if (pos >= argc) {
            std::cerr << "Unexpected end" << std::endl;
			throw std::runtime_error("Unexpected end");
		}
		last = argv[pos];
		return argv[pos++];
	}
};

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
