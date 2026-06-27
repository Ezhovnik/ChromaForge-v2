#include <frontend/locale/langs.h>

#include <utility>

#include <coders/json.h>
#include <coders/BasicParser.h>
#include <content/ContentPack.h>
#include <io/io.h>
#include <util/stringutil.h>
#include <debug/Logger.h>
#include <data/dv.h>

std::unique_ptr<langs::Lang> langs::current;
std::unordered_map<std::string, langs::LocaleInfo> langs::locales_info;

langs::Lang::Lang(std::string locale) : locale(std::move(locale)) {
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

/**
 * @brief Внутренний класс-парсер для файлов переводов.
 *
 * Формат файла: каждая строка содержит ключ и значение, разделённые знаком '='.
 * Поддерживаются комментарии, начинающиеся с '#'.
 * Пустые строки игнорируются.
 */
namespace {
    class Reader : BasicParser<char> {
    public:
        Reader(std::string_view file, std::string_view source)
            : BasicParser(file, source)
        {
            hashComment = true;
        }

        /**
         * @brief Читает файл перевода и заполняет объект Lang.
         * @param lang Целевой объект.
         * @param prefix Префикс, добавляемый к каждому ключу (например, идентификатор мода).
         */
        void read(langs::Lang& lang, const std::string &prefix) {
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
}

void langs::loadLocalesInfo(const io::path& resdir, std::string& fallback) {
    auto file = resdir / langs::TEXTS_FOLDER / "langs.json";
    auto root = io::read_json(file);

    langs::locales_info.clear();
    root.at("fallback").get(fallback);

    if (auto found = root.at("langs")) {
        auto& langs = *found;
        for (const auto& [key, langInfo] : langs.asObject()) {
            std::string name;
            if (langInfo.isObject()) {
                name = langInfo["name"].asString("none");
            } else {
                continue;
            }

            langs::locales_info[key] = LocaleInfo{key, name};
            LOG_DEBUG("Locale {} ({}) added successfully", key, name);
        } 
    }
}

void langs::load(const io::path& resdir, const std::string& locale, const std::vector<ContentPack>& packs, Lang& lang) {
    io::path filename = io::path(TEXTS_FOLDER) / (locale + LANG_FILE_EXT);

    // Сначала загружаем из основных ресурсов
    io::path core_file = resdir / filename;
    if (io::is_regular_file(core_file)) {
        std::string text = io::read_string(core_file);
        Reader reader(core_file.string(), text);
        reader.read(lang, ""); // без префикса
    }

    // Затем загружаем из каждого мода, добавляя префикс (идентификатор мода)
    for (auto pack : packs) {
        io::path file = pack.folder / filename;
        if (io::is_regular_file(file)) {
            std::string text = io::read_string(file);
            Reader reader(file.string(), text);
            reader.read(lang, "");
        }
    }
}

void langs::load(const io::path& resdir, const std::string& locale, const std::string& fallback, const std::vector<ContentPack>& packs) {
    auto lang = std::make_unique<Lang>(locale);
    // Сначала загружаем fallback-локаль (обычно английскую)
    load(resdir, fallback, packs, *lang.get());
    // Если запрошенная локаль отличается от fallback, загружаем и её (переопределяя некоторые строки)
    if (locale != fallback) load(resdir, locale, packs, *lang.get());
    current = std::move(lang);
}

void langs::setup(const io::path& resdir, std::string locale, const std::vector<ContentPack>& packs) {
    std::string fallback = langs::FALLBACK_DEFAULT;
    langs::loadLocalesInfo(resdir, fallback); // Загружаем информацию о доступных локалях из langs.json
    if (langs::locales_info.find(locale) == langs::locales_info.end()) locale = fallback;
    langs::load(resdir, locale, fallback, packs); // Загружаем переводы
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
