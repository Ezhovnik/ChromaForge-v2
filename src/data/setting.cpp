#include <data/setting.h>

#include <util/stringutil.h>

std::string NumberSetting::toString() const {
    switch (getFormat()) {
        case SettingFormat::Simple:
            return util::double2str(value);
        case SettingFormat::Percent:
            return std::to_string(static_cast<integer_t>(round(value * 100))) + "%";
        default:
            return "Invalid format";
    }
}

std::string IntegerSetting::toString() const {
    switch (getFormat()) {
        case SettingFormat::Simple:
            return std::to_string(value);
        case SettingFormat::Percent:
            return std::to_string(value) + "%";
        default:
            return "Invalid format";
    }
}

std::string BoolSetting::toString() const {
    switch (getFormat()) {
        case SettingFormat::Simple:
            return value ? "true" : "false";
        default:
            return "Invalid format";
    }
}

std::string StringSetting::toString() const {
    return value;
}
