#ifndef UTIL_COMMAND_LINE_H_
#define UTIL_COMMAND_LINE_H_

#include <string>
#include <iostream>
#include <stdexcept>
#include <cstring>

#include "../debug/Logger.h"
#include "../files/engine_paths.h"

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

/**
 * @brief Разбирает аргументы командной строки и настраивает пути EnginePaths.
 * @param argc Количество аргументов.
 * @param argv Массив аргументов.
 * @param paths Объект EnginePaths, который будет заполнен.
 * @return true, если движок может продолжить работу, иначе false.
 * @throw std::runtime_error при ошибках (например, указанная папка не существует).
 *
 * Поддерживаемые аргументы:
 * - --res [path] : задаёт папку с ресурсами.
 * - --dir [path] : задаёт папку с пользовательскими данными (создаётся, если не существует).
 * - --help, -h   : выводит справку и завершает разбор.
 */
extern bool parse_cmdline(int argc, char** argv, EnginePaths& paths);

#endif // UTIL_COMMAND_LINE_H_
