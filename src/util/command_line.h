#ifndef UTIL_COMMAND_LINE_H_
#define UTIL_COMMAND_LINE_H_

class EnginePaths;

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
bool parse_cmdline(int argc, char** argv, EnginePaths& paths);

#endif // UTIL_COMMAND_LINE_H_
