#include "langs.h"

#include "../../coders/json.h"
#include "../../coders/commons.h"
#include "../../content/ContentPack.h"
#include "../../files/files.h"
#include "../../util/stringutil.h"
#include "../../logger/Logger.h"

std::unique_ptr<langs::Lang> langs::current;
std::unordered_map<std::string, langs::LocaleInfo> langs::locales_info;

langs::Lang::Lang(std::string locale) : locale(locale) {
}

const std::wstring& langs::Lang::get(const std::wstring& key) const  {
    auto found = map.find(key);
    if (found == map.end()) return key;
    return found->second;
}

void langs::Lang::put(const std::wstring& key, const std::wstring& text) {
    map[key] = text;
}

const std::string& langs::Lang::getId() const {
    return locale;
}

class Reader : public BasicParser {
    void skipWhitespace() override {
        BasicParser::skipWhitespace();
        if (hasNext() && source[pos] == '#') {
            skipLine();
            if (hasNext() && is_whitespace(peek())) skipWhitespace();
        }
    }
public:
    Reader(std::string file, std::string source) : BasicParser(file, source) {}

    void read(langs::Lang& lang, std::string prefix) {
        skipWhitespace();
        while (hasNext()) {
            std::string key = parseString('=', true);
            util::trim(key);
            key = prefix + key;
            std::string text = parseString('\n', false);
            util::trim(text);
            lang.put(util::str2wstr_utf8(key), util::str2wstr_utf8(text));
            skipWhitespace();
        }
    }
};

void langs::loadLocalesInfo(const std::filesystem::path& resdir, std::string& fallback) {
    std::filesystem::path file = resdir/std::filesystem::path(langs::TEXTS_FOLDER)/std::filesystem::path("langs.json");
    std::unique_ptr<json::JObject> root (files::read_json(file));

    langs::locales_info.clear();
    root->str("fallback", fallback);

    auto langs = root->obj("langs");
    if (langs) {
        for (auto& entry : langs->map) {
            auto langInfo = entry.second;

            std::string name;
            if (langInfo->type == json::valtype::object) {
                name = langInfo->value.obj->getStr("name", "none");
            } else {
                continue;
            }

            LOG_DEBUG("Locale {} ({}) added successfully", entry.first, name);
            langs::locales_info[entry.first] = LocaleInfo{entry.first, name};
        } 
    }
}

void langs::load(const std::filesystem::path& resdir, const std::string& locale, const std::vector<ContentPack>& packs, Lang& lang) {
    std::filesystem::path filename = std::filesystem::path(TEXTS_FOLDER)/std::filesystem::path(locale + LANG_FILE_EXT);
    std::filesystem::path core_file = resdir/filename;
    if (std::filesystem::is_regular_file(core_file)) {
        std::string text = files::read_string(core_file);
        Reader reader(core_file.string(), text);
        reader.read(lang, "");
    }
    for (auto pack : packs) {
        std::filesystem::path file = pack.folder/filename;
        if (std::filesystem::is_regular_file(file)) {
            std::string text = files::read_string(file);
            Reader reader(file.string(), text);
            reader.read(lang, pack.id + ":");
        }
    }
}

void langs::load(const std::filesystem::path& resdir, const std::string& locale, const std::string& fallback, const std::vector<ContentPack>& packs) {
    std::unique_ptr<Lang> lang (new Lang(locale));
    load(resdir, fallback, packs, *lang.get());
    load(resdir, locale, packs, *lang.get());
    current.reset(lang.release());
}

void langs::setup(const std::filesystem::path& resdir, const std::string& locale, const std::vector<ContentPack>& packs) {
    std::string fallback = langs::FALLBACK_DEFAULT;
    langs::loadLocalesInfo(resdir, fallback);
    langs::load(resdir, locale, fallback, packs);
}

const std::wstring& langs::get(const std::wstring& key) {
    return current->get(key);
}

const std::wstring& langs::get(const std::wstring& key, const std::wstring& context) {
    std::wstring ctxkey = context + L"." + key;
    const std::wstring& text = current->get(ctxkey);
    if (&ctxkey != &text) return text;
    return current->get(key);
}
