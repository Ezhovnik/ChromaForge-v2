#pragma once

#include <string>
#include <vector>

#include <typedefs.h>

namespace util {
    std::string escape(const std::string& s);

    std::string quote(const std::string& s);

    std::wstring lfill(std::wstring s, uint length, wchar_t c);
    std::wstring rfill(std::wstring s, uint length, wchar_t c);

    uint encode_utf8(uint32_t c, ubyte* bytes);
    uint32_t decode_utf8(uint& size, const char* bytes);
    std::string wstr2str_utf8(const std::wstring &ws);
    std::wstring str2wstr_utf8(const std::string &s);
    size_t crop_utf8(std::string_view s, size_t maxSize);
    std::string double2str(double x);
    std::wstring double2wstr(double x, int precision);
    std::string u32str2str_utf8(const std::u32string& ws);
    std::u32string str2u32str_utf8(const std::string& s);
    bool is_integer(const std::string& text);
    bool is_integer(const std::wstring& text);
    bool is_valid_filename(const std::wstring& name);

    int replaceAll(std::string& str, const std::string& from, const std::string& to);

    double parse_double(const std::string& str);
    double parse_double(const std::string& str, size_t offset, size_t len);

    void ltrim(std::string &s);
    void rtrim(std::string &s);
    void trim(std::string &s);

    std::string base64_encode(const ubyte* data, size_t size);
    std::vector<ubyte> base64_decode(const char* str, size_t size);
    std::vector<ubyte> base64_decode(const std::string& str);

    std::string mangleid(uint64_t value);

    std::wstring lower_case(const std::wstring& str);
    std::wstring upper_case(const std::wstring& str);
    std::wstring capitalized(const std::wstring& str);
    std::wstring pascal_case(const std::wstring& str);

    std::string id_to_caption(const std::string& id);

    std::vector<std::string> split(const std::string& str, char delimiter);
    std::vector<std::wstring> split(const std::wstring& str, char delimiter);
    std::pair<std::string, std::string> split_at(std::string_view view, char c);

    std::string format_data_size(size_t size);
}
