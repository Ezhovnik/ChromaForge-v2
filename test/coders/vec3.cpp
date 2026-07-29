#include <gtest/gtest.h>

#include <coders/vec3.h>
#include <io/io.h>
#include <io/devices/StdfsDevice.h>

namespace fs = std::filesystem;

TEST(VEC3, Decode) {
    io::set_device("res", std::make_shared<io::StdfsDevice>(fs::u8path("res")));
    io::path file = "res:models/block.vec3";

    ASSERT_TRUE(fs::exists("res/models/block.vec3"));

    auto bytes = io::read_bytes_buffer(file);
    auto model = vec3::load(file.string(), bytes);
}
