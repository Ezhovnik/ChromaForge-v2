#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <util/ArgsReader.h>

inline std::filesystem::path TESTING_DIR = std::filesystem::u8path(".vctest");

struct Config {
    std::filesystem::path executable;
    std::filesystem::path directory;
    std::filesystem::path resDir {"res"};
    std::filesystem::path workingDir {"."};
    std::string memchecker = "valgrind";
    bool outputAlways = false;
};

static bool perform_keyword(
    util::ArgsReader& reader, const std::string& keyword, Config& config
) {
    if (keyword == "--help" || keyword == "-h") {
        std::cout << "Options\n\n";
        std::cout << "  --help, -h                      = show help\n";
        std::cout << "  --exe <path>, -e <path>         = ChromaForge executable path\n";
        std::cout << "  --tests <path>, -d <path>       = tests directory path\n";
        std::cout << "  --res <path>, -r <path>         = 'res' directory path\n";
        std::cout << "  --user <path>, -u <path>        = user directory path\n";
        std::cout << "  --memchecker <path>             = path to valgrind\n";
        std::cout << "  --output-always                 = always show tests output\n";
        std::cout << std::endl;
        return false;
    } else if (keyword == "--exe" || keyword == "-e") {
        config.executable = std::filesystem::path(reader.next());
    } else if (keyword == "--tests" || keyword == "-d") {
        config.directory = std::filesystem::path(reader.next());
    } else if (keyword == "--res" || keyword == "-r") {
        config.resDir = std::filesystem::path(reader.next());
    } else if (keyword == "--user" || keyword == "-u") {
        config.workingDir = std::filesystem::path(reader.next());
    } else if (keyword == "--output-always") {
        config.outputAlways = true;
    } else if (keyword == "--memchecker") {
        config.memchecker = reader.next();
    } else {
        std::cerr << "Unknown argument " << keyword << std::endl;
        return false;
    }
    return true;
}

static bool parse_cmdline(int argc, char** argv, Config& config) {
    util::ArgsReader reader(argc, argv);
    while (reader.hasNext()) {
        std::string token = reader.next();
        if (reader.isKeywordArg()) {
            if (!perform_keyword(reader, token, config)) {
                return false;
            }
        }
    }
    return true;
}

static bool check_dir(const std::filesystem::path& dir) {
    if (!std::filesystem::is_directory(dir)) {
        std::cerr << dir << " is not a directory" << std::endl;
        return false;
    }
    return true;
}

static void print_separator(std::ostream& stream) {
    for (int i = 0; i < 32; i++) {
        stream << "=";
    }
    stream << "\n";
}

static bool check_config(const Config& config) {
    if (!std::filesystem::exists(config.executable)) {
        std::cerr << "file " << config.executable << " not found" << std::endl;
        return true;
    }
    if (!check_dir(config.directory)) {
        return true;
    }
    if (!check_dir(config.resDir)) {
        return true;
    }
    if (!check_dir(config.workingDir)) {
        return true;
    }
    return false;
}

static void dump_config(const Config& config) {
    std::cout << "paths:\n";
    std::cout << "  ChromaForge executable = " << std::filesystem::canonical(config.executable).string() << "\n";
    std::cout << "  Tests directory      = " << std::filesystem::canonical(config.directory).string() << "\n";
    std::cout << "  Resources directory  = " << std::filesystem::canonical(config.resDir).string() << "\n";
    std::cout << "  Working directory    = " << std::filesystem::canonical(config.workingDir).string();
    std::cout << std::endl;
}

static void cleanup(const std::filesystem::path& dir) {
    std::cout << "Cleaning up " << dir << std::endl;
    std::filesystem::remove_all(dir);
}

static void setup_working_dir(const std::filesystem::path& workingDir) {
    auto dir = workingDir / TESTING_DIR;
    std::cout << "Setting up working directory " << dir << std::endl;
    if (std::filesystem::is_directory(dir)) {
        cleanup(dir);
    }
    std::filesystem::create_directories(dir);
}

static void display_test_output(
    const std::filesystem::path& path, const std::filesystem::path& name, std::ostream& stream
) {
    stream << "[OUTPUT]" << name << std::endl;
    if (std::filesystem::exists(path)) {
        std::ifstream t(path);
        stream << t.rdbuf();
    }
}

static void display_segfault_valgrind(
    const std::filesystem::path& path, const std::filesystem::path& name, std::ostream& stream
) {
    stream << "[MEMCHECK] " << name << std::endl;
    if (std::filesystem::exists(path)) {
        std::ifstream t(path);
        while (!t.eof()) {
            std::string line;
            std::getline(t, line);
            if (line.find("Process terminating with default action of signal ") != std::string::npos) {
                break;
            }
        }
        std::stringstream ss;
        while (!t.eof()) {
            std::string line;
            std::getline(t, line);
            size_t pos = line.find("== ");
            if (pos == std::string::npos) {
                continue;
            }
            if (line.find("If you") != std::string::npos || line.find("HEAP SUMMARY:") != std::string::npos) {
                break;
            }
            ss << line.substr(pos + 3) << "\n";
        }
        stream << ss.str() << std::endl;
    }
}

static std::string fix_path(std::string s) {
    for (char& c : s) {
        if (c == '\\') {
            c = '/';
        }
    }
    return s;
}

static bool run_test(const Config& config, const std::filesystem::path& path, bool memcheck = false) {
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    auto outputFile = config.workingDir / "output.txt";
    auto memcheckLogFile = config.workingDir / "memcheck.txt";

    auto name = path.stem();
    std::stringstream ss;
    if (memcheck) ss << config.memchecker << " --log-file=" << fix_path(memcheckLogFile.string()) << " ";
    ss << "\"" << std::filesystem::canonical(config.executable).string() << "\" --headless";
    ss << " --test \"" << fix_path(path.string()) << "\"";
    ss << " --res \"" << fix_path(config.resDir.string()) << "\"";
    ss << " --dir \"" << fix_path(config.workingDir.string()) << "\"";
    ss << " >" << fix_path(outputFile.string()) << " 2>&1";
    auto command = ss.str();

    print_separator(std::cout);
    std::cout << "Executing test " << name << "\nCommand: " << command << std::endl;

    auto start = high_resolution_clock::now();
    // FIXME: only for Windows!
    std::string wrapped = "cmd /c \"" + command + "\"";
    int code = system(wrapped.c_str());
    auto testTime = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();

    if (code) {
        if (memcheck) {
            display_segfault_valgrind(memcheckLogFile, name, std::cerr);
            std::filesystem::remove(memcheckLogFile);
            std::filesystem::remove(outputFile);
        } else {
            display_test_output(outputFile, name, std::cerr);
            std::cerr << "[FAILED] " << name << " in " << testTime << " ms (code=" << code << ")" << std::endl;
            std::filesystem::remove(outputFile);
            run_test(config, path, true);
        }
        return false;
    } else {
        if (config.outputAlways) {
            display_test_output(outputFile, name, std::cout);
        }
        std::cout << "[PASSED] " << name << " in " << testTime << " ms" << std::endl;
        std::filesystem::remove(outputFile);
        return true;
    }
}

int main(int argc, char** argv) {
    Config config;
    try {
        if (!parse_cmdline(argc, argv, config)) {
            return 0;
        }
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        throw;
    }
    if (check_config(config)) {
        return 1;
    }
    dump_config(config);
    std::vector<std::filesystem::path> tests;
    std::cout << "Scanning for tests" << std::endl;
    for (const auto& entry : std::filesystem::directory_iterator(config.directory)) {
        auto path = entry.path();
        if (path.extension().string() != ".lua") {
            std::cout << "  " << entry.path() << " skipped" << std::endl;
            continue;
        }
        std::cout << "  " << entry.path() << " enqueued" << std::endl;
        tests.push_back(path);
    }

    setup_working_dir(config.workingDir);
    config.workingDir /= TESTING_DIR;

    size_t passed = 0;
    std::cout << "Running " << tests.size() << " test(s)" << std::endl;
    for (const auto& path : tests) {
        passed += run_test(config, path);
        std::filesystem::remove_all(config.workingDir/std::filesystem::u8path("saves"));
    }
    print_separator(std::cout);
    cleanup(config.workingDir);
    std::cout << std::endl;
    std::cout << passed << " test(s) passed, " << (tests.size() - passed) << " test(s) failed" << std::endl;
    if (passed < tests.size()) {
        return 1;
    }
    return 0;
}
