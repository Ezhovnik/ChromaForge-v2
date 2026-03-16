#include "setting.h"

#include "../util/stringutil.h"

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
