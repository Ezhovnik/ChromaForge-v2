#include "command_line.h"

#include <filesystem>

bool parse_cmdline(int argc, char** argv, EnginePaths& paths) {
	ArgsReader reader(argc, argv);
	reader.skip();
	while (reader.hasNext()) {
		std::string token = reader.next();
		if (reader.isKeywordArg()) {
			if (token == "--res") {
				token = reader.next();
				if (!std::filesystem::is_directory(std::filesystem::path(token))) {
					throw std::runtime_error(token + " is not a directory");
				}
				paths.setResources(std::filesystem::path(token));
				std::cout << "Resources folder: " << token << std::endl;
			} else if (token == "--dir") {
				token = reader.next();
				if (!std::filesystem::is_directory(std::filesystem::path(token))) {
					std::filesystem::create_directories(std::filesystem::path(token));
				}
				paths.setUserfiles(std::filesystem::path(token));
				std::cout << "Userfiles folder: " << token << std::endl;
			} else if (token == "--help" || token == "-h") {
				std::cout << "ChromaForge command-line arguments:" << std::endl;
				std::cout << " --res [path] - set resources directory" << std::endl;
				std::cout << " --dir [path] - set userfiles directory" << std::endl;
				return false;
			} else {
				std::cerr << "Unknown argument " << token << std::endl;
			}
		} else {
			std::cerr << "Unexpected token" << std::endl;
		}
	}
	return true;
}
