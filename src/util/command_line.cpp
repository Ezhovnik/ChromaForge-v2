#include <util/command_line.h>

#include <filesystem>
#include <iostream>
#include <iomanip>

#include <engine/EnginePaths.h>
#include <engine/Engine.h>
#include <util/ArgsReader.h>

class ArgC {
public:
    std::string keyword;
    std::function<bool(CoreParameters&, util::ArgsReader&)> execute;
    std::string args;
    std::string help;
    ArgC(
        const std::string& keyword,
        std::function<bool(CoreParameters&, util::ArgsReader&)> execute,
        const std::string& args,
        const std::string& help
    ) {
        this->keyword = keyword;
        this->execute = execute;
        this->args = args;
        this->help = help;
    }
};

static bool perform_keyword(
    util::ArgsReader& reader, const std::string& keyword, CoreParameters& params
) {
    static const std::vector<ArgC> argumentsCommandline = {
        ArgC("--res", [](auto& params, auto& reader) -> bool {
            params.resFolder = reader.next();
            return true;
        }, "<path>", "set resources directory."),
        ArgC("--dir", [](auto& params, auto& reader) -> bool {
            params.userFolder = reader.next();
            return true;
        }, "<path>", "set userfiles directory."),
        ArgC("--project", [](auto& params, auto& reader) -> bool {
            params.projectFolder = reader.next();
            return true;
        }, "<path>", "set project directory."),
        ArgC("--test", [](auto& params, auto& reader) -> bool {
            params.testMode = true;
            params.scriptFile = reader.next();
            return true;
        }, "<path>", "test script file."),
        ArgC("--script", [](auto& params, auto& reader) -> bool {
            params.testMode = false;
            params.scriptFile = reader.next();
            return true;
        }, "<path>", "main script file."),
        ArgC("--headless", [](auto& params, auto& reader) -> bool {
            params.headless = true;
            return true;
        }, "", "run in headless mode."),
        ArgC("--stdin-cmd", [](auto& params, auto& reader) -> bool {
            params.stdinCommands = true;
            return true;
        }, "", "run commands from stdin."),
        ArgC("--sps", [](auto& params, auto& reader) -> bool {
            params.sps = reader.nextInt();
            return true;
        }, "<sps>", "headless mode spark(tick)-rate (default - 20)."),
        ArgC("--version", [](auto&, auto&) -> bool {
            std::cout << ENGINE_VERSION_STRING << std::endl;
            return false;
        }, "", "display the engine version."),
        ArgC("--dbg-server", [](auto& params, auto& reader) -> bool {
            params.debugServerString = reader.next();
            return true;
        }, "<serv>", "open debugging server where <serv> is {transport}:{port}"),
        ArgC("--sub-depth", [](auto& params, auto& reader) -> bool {
            params.subProcessDepth = reader.nextInt();
            return true;
        }, "<depth>", "sub-process depth"),
        ArgC("--help", [](auto&, auto&) -> bool {
            std::cout << "ChromaForge v" << ENGINE_VERSION_STRING << "\n\n";
            std::cout << "Command-line arguments:\n";
            for (auto& arg : argumentsCommandline) {
                if (arg.help.empty()) {
                    continue;
                }
                std::cout << std::setw(24) << std::left << (arg.keyword + " " + arg.args);
                std::cout << "- " << arg.help << std::endl;
            }
            std::cout << std::endl;
            return false;
        }, "", "display this help.")
    };
    for (auto& a : argumentsCommandline) {
        if (a.keyword == keyword) {
            return a.execute(params, reader);
        }
    }
    throw std::runtime_error("Unknown argument " + keyword);
}

static void parse_project_args(
    util::ArgsReader& reader, CoreParameters& params
) {
    while (reader.hasNext()) {
        std::string key = reader.next();
        if (!reader.isKeywordArg()) {
            std::cerr << "--<keyword> argument expected" << std::endl;
            return;
        }
        key = key.substr(2);
        if (!reader.hasNext() || reader.isNextKeywordArg()) {
            params.projectArgs[std::move(key)] = "";
            continue;
        }
        std::string value = reader.next();
        params.projectArgs[std::move(key)] = std::move(value);
    }
}

bool parse_cmdline(int argc, char** argv, CoreParameters& params) {
    util::ArgsReader reader(argc, argv);
    reader.skip(); // пропускаем имя исполняемого файла (argv[0])
    while (reader.hasNext()) {
        std::string token = reader.next();
        if (token == "--") {
            parse_project_args(reader, params);
            return true;
        }
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
