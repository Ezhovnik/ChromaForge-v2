#include <io/devices/StdfsDevice.h>

#include <fstream>
#include <filesystem>

#include <debug/Logger.h>

using namespace io;

StdfsDevice::StdfsDevice(std::filesystem::path root, bool createDirectory) : root(std::move(root)) {
    if (createDirectory && !std::filesystem::is_directory(this->root)) {
        std::error_code ec;
        std::filesystem::create_directories(this->root, ec);
        if (ec) {
            LOG_ERROR("Error creating root directory {}: {}", this->root.u8string(), ec.message());
        }
    }
}

std::filesystem::path StdfsDevice::resolve(std::string_view path) {
    return root / std::filesystem::u8path(io::path(std::string(path)).normalized().string());
}

std::unique_ptr<std::ostream> StdfsDevice::write(std::string_view path) {
    auto resolved = resolve(path);
    auto output = std::make_unique<std::ofstream>(resolved, std::ios::binary);
    if (!output->is_open()) {
        LOG_ERROR("Could not to open file {}", resolved.u8string());
        throw std::runtime_error("Could not to open file " + resolved.u8string());
    }
    return output;
}

std::unique_ptr<std::istream> StdfsDevice::read(std::string_view path) {
    auto resolved = resolve(path);
    auto input = std::make_unique<std::ifstream>(resolved, std::ios::binary);
    if (!*input) {
        LOG_ERROR("Could not to open file {}", resolved.u8string());
        throw std::runtime_error("Could not to open file " + resolved.u8string());
    }
    return input;
}

size_t StdfsDevice::size(std::string_view path) {
    return std::filesystem::file_size(resolve(path));
}

file_time_type StdfsDevice::lastWriteTime(std::string_view path) {
    return std::filesystem::last_write_time(resolve(path));
}

bool StdfsDevice::exists(std::string_view path) {
    return std::filesystem::exists(resolve(path));
}

bool StdfsDevice::isdir(std::string_view path) {
    return std::filesystem::is_directory(resolve(path));
}

bool StdfsDevice::isfile(std::string_view path) {
    return std::filesystem::is_regular_file(resolve(path));
}

bool StdfsDevice::mkdir(std::string_view path) {
    auto resolved = resolve(path);

    std::error_code ec;
    bool created = std::filesystem::create_directory(resolved, ec);
    if (ec) {
        LOG_ERROR("Error creating directory {}: {}", resolved.u8string(), ec.message());
    }
    return created;
}

bool StdfsDevice::mkdirs(std::string_view path) {
    auto resolved = resolve(path);

    std::error_code ec;
    bool created = std::filesystem::create_directories(resolved, ec);
    if (ec) {
        LOG_ERROR("Error creating directories {}: {}", resolved.u8string(), ec.message());
    }
    return created;
}

bool StdfsDevice::remove(std::string_view path) {
    auto resolved = resolve(path);
    return std::filesystem::remove(resolved);
}

uint64_t StdfsDevice::removeAll(std::string_view path) {
    auto resolved = resolve(path);
    if (std::filesystem::exists(resolved)) {
        LOG_INFO("removeALL {}", resolved.u8string());
        return std::filesystem::remove_all(resolved);
    } else {
        return 0;
    }
}

class StdfsPathsGenerator : public PathsGenerator {
public:
    StdfsPathsGenerator(std::filesystem::path root) : root(std::move(root)) {
        it = std::filesystem::directory_iterator(this->root);
    }

    bool next(io::path& path) override {
        if (it == std::filesystem::directory_iterator()) {
            return false;
        }
        path = it->path().filename().u8string();
        ++it;
        return true;
    }
private:
    std::filesystem::path root;
    std::filesystem::directory_iterator it;
};

std::unique_ptr<PathsGenerator> StdfsDevice::list(std::string_view path) {
    return std::make_unique<StdfsPathsGenerator>(resolve(path));
}
