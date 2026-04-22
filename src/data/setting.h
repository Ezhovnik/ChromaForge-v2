#pragma once

#include <string>
#include <limits>
#include <vector>
#include <unordered_set>

#include <typedefs.h>
#include <delegates.h>

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

template<class T>
class ObservableSetting : public Setting {
private:
    int nextid = 1;
    std::unordered_map<int, consumer<T>> observers;
protected:
    T initial;
    T value;
public:
    ObservableSetting(T value, SettingFormat format) : Setting(format), initial(value), value(value) {}

    observer_handler observe(consumer<T> callback, bool callOnStart=false) {
        const int id = nextid++;
        observers.emplace(id, callback);
        if (callOnStart) callback(value);
        return std::shared_ptr<int>(new int(id), [this](int* id) {
            observers.erase(*id);
            delete id;
        });
    }

    const T& get() const {
        return value;
    }

    T& operator*() {
        return value;
    }

    void notify(T value) {
        for (auto& entry : observers) {
            entry.second(value);
        }
    }

    void set(T value) {
        if (value == this->value) return;
        this->value = value;
        notify(value);
    }

    virtual void resetToDefault() override {
        set(initial);
    }
};

class NumberSetting : public ObservableSetting<number_t> {
protected:
    number_t min;
    number_t max;
public:
    NumberSetting(
        number_t value, 
        number_t min=std::numeric_limits<number_t>::min(), 
        number_t max=std::numeric_limits<number_t>::max(),
        SettingFormat format=SettingFormat::Simple
    ) : ObservableSetting(value, format),
        min(min), 
        max(max) 
    {}

    number_t getMin() const {
        return min;
    }

    number_t getMax() const {
        return max;
    }

    number_t getT() const {
        return (value - min) / (max - min);
    }

    virtual std::string toString() const override;

    static inline NumberSetting createPercent(number_t def) {
        return NumberSetting(def, 0.0, 1.0, SettingFormat::Percent);
    }
};

class IntegerSetting : public ObservableSetting<integer_t> {
protected:
    integer_t min;
    integer_t max;
public:
    IntegerSetting(
        integer_t value, 
        integer_t min=std::numeric_limits<integer_t>::min(), 
        integer_t max=std::numeric_limits<integer_t>::max(),
        SettingFormat format=SettingFormat::Simple
    ) : ObservableSetting(value, format),
        min(min), 
        max(max) {}

    integer_t getMin() const {
        return min;
    }

    integer_t getMax() const {
        return max;
    }

    integer_t getT() const {
        return (value - min) / (max - min);
    }

    virtual std::string toString() const override;
};

class BoolSetting : public ObservableSetting<bool> {
public:
    BoolSetting(
        bool value,
        SettingFormat format=SettingFormat::Simple
    ) : ObservableSetting(value, format) {}

    void toggle() {
        set(!get());
    }

    virtual std::string toString() const override;
};

class StringSetting : public ObservableSetting<std::string> {
public:
    StringSetting(
        std::string value,
        SettingFormat format=SettingFormat::Simple
    ) : ObservableSetting(value, format) {}

    virtual std::string toString() const override;
};
