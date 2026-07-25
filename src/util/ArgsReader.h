#pragma once

#include <string>
#include <stdexcept>
#include <cstring>

namespace util {
    /**
     * @brief Утилита для последовательного чтения аргументов командной строки.
     *
     * Позволяет проходить по аргументам, проверять наличие следующего,
     * определять, является ли текущий аргумент ключом (начинается с '-'),
     * и получать значение аргумента.
     */
    class ArgsReader {
    private:
        const char* last = ""; ///< Последний прочитанный аргумент
        char** argv; ///< Массив аргументов
        int argc; ///< Количество аргументов
        int pos = 0;
    public:
        /**
         * @brief Конструктор.
         * @param argc Количество аргументов (из main).
         * @param argv Массив аргументов (из main).
         */
        ArgsReader(int argc, char** argv) : argv(argv), argc(argc) {}

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
                throw std::runtime_error("Unexpected end");
            }
            last = argv[pos];
            return argv[pos++];
        }

        int nextInt() {
            auto text = next();
            try {
                return std::stoi(text);
            } catch (const std::exception& e) {
                throw std::runtime_error(e.what());
            }
        }
    };
}
