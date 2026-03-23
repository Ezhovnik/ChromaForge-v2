#ifndef FILES_SETTINGS_IO_H_
#define FILES_SETTINGS_IO_H_

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "../data/dynamic.h"

class Setting;
struct EngineSettings;

struct Section {
    std::string name;
    std::vector<std::string> keys;
};

class SettingsHandler {
private:
    std::unordered_map<std::string, Setting*> map;
    std::vector<Section> sections;
public:
    SettingsHandler(EngineSettings& settings);

    std::unique_ptr<dynamic::Value> getValue(const std::string& name) const;
    void setValue(const std::string& name, const dynamic::Value& value);

    std::string toString(const std::string& name) const;

    Setting* getSetting(const std::string& name) const;

    std::vector<Section>& getSections();
};

#endif // FILES_SETTINGS_IO_H_
