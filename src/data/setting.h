#ifndef DATA_SETTING_H_
#define DATA_SETTING_H_

#include <string>
#include <limits>
#include <vector>

#include "../typedefs.h"
#include "../delegates.h"

enum class SettingFormat {
    Simple,
    Percent
};

class Setting {
protected:
    SettingFormat format;
public:
    Setting(SettingFormat format) : format(format) {}

    virtual ~Setting() {}

    virtual void resetToDefault() = 0;

    virtual SettingFormat getFormat() const {
        return format;
    }

    virtual std::string toString() const = 0;
};

class NumberSetting : public Setting {
protected:
    number_t initial;
    number_t value;
    number_t min;
    number_t max;
    std::vector<consumer<number_t>> consumers;
public:
    NumberSetting(
        number_t value, 
        number_t min=std::numeric_limits<number_t>::min(), 
        number_t max=std::numeric_limits<number_t>::max(),
        SettingFormat format=SettingFormat::Simple
    ) : Setting(format), 
        initial(value), 
        value(value), 
        min(min), 
        max(max) 
    {}

    number_t& operator*() {
        return value;
    }

    number_t get() const {
        return value;
    }

    void set(number_t value) {
        if (value == this->value) return;

        this->value = value;
        for (auto& callback : consumers) {
            callback(value);
        }
    }

    number_t getMin() const {
        return min;
    }

    number_t getMax() const {
        return max;
    }

    number_t getT() const {
        return (value - min) / (max - min);
    }

    virtual void resetToDefault() override {
        value = initial;
    }

    void observe(consumer<number_t> callback) {
        consumers.push_back(callback);
    }

    virtual std::string toString() const override;

    static inline NumberSetting createPercent(number_t def) {
        return NumberSetting(def, 0.0, 1.0, SettingFormat::Percent);
    }
};

#endif // DATA_SETTING_H_
