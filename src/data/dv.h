#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <unordered_map>
#include <iosfwd>

namespace util {
    template<class T> class Buffer;
}

namespace dv {
    using integer_t = int64_t;
    using number_t = double;
    using boolean_t = bool;
    using byte_t = unsigned char;
    using key_t = std::string;

    enum class value_type : uint8_t {
        None = 0,
        Number,
        Boolean,
        Integer,
        Object,
        List,
        Bytes,
        String
    };

    inline const std::string& type_name(value_type type) {
        static std::string type_names[] = {
            "none",
            "number",
            "boolean",
            "integer",
            "object",
            "list",
            "bytes",
            "string"
        };
        return type_names[static_cast<int>(type)];
    }

    class value;

    using list_t = std::vector<value>;
    using map_t = std::unordered_map<key_t, value>;
    using pair = std::pair<const key_t, value>;

    using reference = value&;
    using const_reference = const value&;

    namespace objects {
        using Object = std::unordered_map<key_t, value>;
        using List = std::vector<value>;
        using Bytes = util::Buffer<byte_t>;
    }

    struct optionalvalue {
        value* ptr;

        optionalvalue(value* ptr) noexcept : ptr(ptr) {}

        inline operator bool() const noexcept {
            return ptr != nullptr;
        }

        inline value& operator*() noexcept {
            return *ptr;
        }

        inline const value& operator*() const noexcept {
            return *ptr;
        }

        bool get(std::string& dst) const;
        bool get(bool& dst) const;
        bool get(char& dst) const;
        bool get(short& dst) const;
        bool get(int& dst) const;
        bool get(long& dst) const;
        bool get(long long& dst) const;
        bool get(unsigned char& dst) const;
        bool get(unsigned short& dst) const;
        bool get(unsigned int& dst) const;
        bool get(unsigned long& dst) const;
        bool get(unsigned long long& dst) const;
        bool get(float& dst) const;
        bool get(double& dst) const;
    };

    void log_error(const std::string& log);

    inline void throw_type_error(value_type got, value_type expected) {
        log_error("Type error: expected " + type_name(expected) + ", got " + type_name(got));
        throw std::runtime_error(
            "Type error: expected " + type_name(expected) + ", got " + type_name(got)
        );
    }

    inline void check_type(value_type got, value_type expected) {
        if (got != expected) {
            throw_type_error(got, expected);
        }
    }

    class value {
        value_type type = value_type::None;
        union value_u {
            integer_t integer;
            number_t number;
            boolean_t boolean;
            std::unique_ptr<std::string> string;
            std::shared_ptr<objects::Object> object;
            std::shared_ptr<objects::List> list;
            std::shared_ptr<objects::Bytes> bytes;
            value_u() noexcept {}
            ~value_u() noexcept {}
        } val;

        inline value& setBoolean(boolean_t v) noexcept {
            this->~value();
            type = value_type::Boolean;
            val.boolean = v;
            return *this;
        }
        inline value& setInteger(integer_t v) noexcept {
            this->~value();
            type = value_type::Integer;
            val.integer = v;
            return *this;
        }
        inline value& setNumber(number_t v) noexcept {
            this->~value();
            type = value_type::Number;
            val.number = v;
            return *this;
        }
        inline value& setNone() noexcept {
            this->~value();
            type = value_type::None;
            return *this;
        }
        inline value& setString(std::string v) noexcept {
            this->~value();
            new(&val.string)std::unique_ptr<std::string>(std::make_unique<std::string>(std::move(v)));
            type = value_type::String;
            return *this;
        }
        inline value& setString(std::unique_ptr<std::string> v) noexcept {
            this->~value();
            new(&val.string)std::unique_ptr<std::string>(std::move(v));
            type = value_type::String;
            return *this;
        }
        inline value& setList(std::shared_ptr<objects::List> ptr) noexcept {
            this->~value();
            new(&val.list)std::shared_ptr<objects::List>(std::move(ptr));
            type = value_type::List;
            return *this;
        }
        inline value& setObject(std::shared_ptr<objects::Object> ptr) noexcept {
            this->~value();
            new(&val.object)std::shared_ptr<objects::Object>(std::move(ptr));
            type = value_type::Object;
            return *this;
        }
        inline value& setBytes(std::shared_ptr<objects::Bytes> ptr) noexcept {
            this->~value();
            new(&val.bytes)std::shared_ptr<objects::Bytes>(std::move(ptr));
            type = value_type::Bytes;
            return *this;
        }
    public:
        value() noexcept : type(value_type::None) {}

        template<typename T>
        value(T v, std::enable_if_t<std::is_fundamental<T>::value, int> = 0) noexcept {
            this->operator=(v);
        }
        value(std::string v) noexcept {
            this->operator=(std::move(v));
        }
        value(std::shared_ptr<objects::Object> v) noexcept {
            this->operator=(std::move(v));
        }
        value(std::shared_ptr<objects::List> v) noexcept {
            this->operator=(std::move(v));
        }
        value(std::shared_ptr<objects::Bytes> v) noexcept {
            this->operator=(std::move(v));
        }

        value(const value& v) noexcept : type(value_type::None) {
            this->operator=(v);
        }

        value(value&& v) noexcept {
            this->operator=(std::move(v));
        }

        ~value() noexcept {
            switch (type) {
                case value_type::Object:
                    val.object.reset();
                    break;
                case value_type::List:
                    val.list.reset();
                    break;
                case value_type::Bytes:
                    val.bytes.reset();
                    break;
                case value_type::String:
                    val.string.reset();
                    break;
                default:
                    break;
            }
        }

        inline value& operator=(std::nullptr_t) {
            return setNone();
        }

        template<typename T>
        inline std::enable_if_t<std::is_integral<T>() && !std::is_same<T, bool>(), value&>operator=(T v) {
            return setInteger(v);
        }
        inline value& operator=(float v) {
            return setNumber(v);
        }
        inline value& operator=(double v) {
            return setNumber(v);
        }
        inline value& operator=(bool v) {
            return setBoolean(v);
        }
        inline value& operator=(std::string_view v) {
            return setString(std::string(v));
        }
        inline value& operator=(std::string v) {
            return setString(std::move(v));
        }
        inline value& operator=(const char* v) {
            return setString(v);
        }
        inline value& operator=(std::shared_ptr<objects::List> ptr) {
            return setList(std::move(ptr));
        }
        inline value& operator=(std::shared_ptr<objects::Object> ptr) {
            return setObject(std::move(ptr));
        }
        inline value& operator=(std::shared_ptr<objects::Bytes> ptr) {
            return setBytes(std::move(ptr));
        }
        value& operator=(const objects::Bytes& bytes);

        inline value& operator=(const value& v) {
            switch (v.type) {
                case value_type::Object:
                    setObject(v.val.object);
                    break;
                case value_type::List:
                    setList(v.val.list);
                    break;
                case value_type::Bytes:
                    setBytes(v.val.bytes);
                    break;
                case value_type::String:
                    setString(*v.val.string);
                    break;
                case value_type::Boolean:
                    setBoolean(v.val.boolean);
                    break;
                case value_type::Integer:
                    setInteger(v.val.integer);
                    break;
                case value_type::Number:
                    setNumber(v.val.number);
                    break;
                case value_type::None:
                    setNone();
                    break;
            }
            return *this;
        }

        inline value& operator=(value&& v) noexcept {
            if (type < value_type::Object) {
                type = v.type;
                switch (v.type) {
                    case value_type::None:
                        break;
                    case value_type::Integer:
                        val.integer = v.val.integer;
                        break;
                    case value_type::Number:
                        val.number = v.val.number;
                        break;
                    case value_type::Boolean:
                        val.boolean = v.val.boolean;
                        break;
                    case value_type::String:
                        new(&val.string)std::unique_ptr<std::string>(
                            std::move(v.val.string));
                        break;
                    case value_type::Object:
                        new(&val.object)std::shared_ptr<objects::Object>(
                            std::move(v.val.object));
                        break;
                    case value_type::List:
                        new(&val.list)std::shared_ptr<objects::List>(
                            std::move(v.val.list));
                        break;
                    case value_type::Bytes:
                        new(&val.list)std::shared_ptr<objects::Bytes>(
                            std::move(v.val.bytes));
                        break;
                }
            } else {
                switch (v.type) {
                    case value_type::Object:
                        setObject(std::move(v.val.object));
                        break;
                    case value_type::List:
                        setList(std::move(v.val.list));
                        break;
                    case value_type::Bytes:
                        setBytes(std::move(v.val.bytes));
                        break;
                    case value_type::String:
                        setString(std::move(v.val.string));
                        break;
                    case value_type::Boolean:
                        setBoolean(v.val.boolean);
                        break;
                    case value_type::Integer:
                        setInteger(v.val.integer);
                        break;
                    case value_type::Number:
                        setNumber(v.val.number);
                        break;
                    case value_type::None:
                        setNone();
                        break;
                }
            }
            return *this;
        }

        void add(value v);

        template<class T>
        inline void add(T v) {
            return add(value(v));
        }

        void erase(const key_t& key);

        void erase(size_t index);

        value& operator[](const key_t& key);

        const value& operator[](const key_t& key) const;

        value& operator[](size_t index);

        const value& operator[](size_t index) const;

        bool operator!=(std::nullptr_t) const noexcept {
            return type != value_type::None;
        }

        bool operator==(std::nullptr_t) const noexcept {
            return type == value_type::None;
        }

        value& object(const key_t& key);

        value& list(const key_t& key);

        value& object();

        value& list();

        list_t::iterator begin();
        list_t::iterator end();

        list_t::const_iterator begin() const;
        list_t::const_iterator end() const;

        const std::string& asString() const;

        integer_t asInteger() const;

        number_t asNumber() const;

        boolean_t asBoolean() const;

        objects::Bytes& asBytes();

        const objects::Bytes& asBytes() const;   

        const objects::Object& asObject() const;

        inline value_type getType() const {
            return type;
        }

        std::string asString(std::string def) const {
            if (type != value_type::String) {
                return def;
            }
            return *val.string;
        }

        std::string asString(const char* s) const {
            return asString(std::string(s));
        }

        integer_t asBoolean(boolean_t def) const {
            switch (type) {
                case value_type::Boolean: 
                    return val.boolean;
                default:
                    return def;
            }
        }

        integer_t asInteger(integer_t def) const {
            switch (type) {
                case value_type::Integer: 
                    return val.integer;
                case value_type::Number: 
                    return static_cast<integer_t>(val.number);
                default:
                    return def;
            }
        }

    integer_t asNumber(integer_t def) const {
            switch (type) {
                case value_type::Integer: 
                    return static_cast<number_t>(val.integer);
                case value_type::Number: 
                    return val.number;
                default:
                    return def;
            }
        }

        optionalvalue at(const key_t& k) const {
            check_type(type, value_type::Object);
            const auto& found = val.object->find(k);
            if (found == val.object->end()) {
                return optionalvalue(nullptr);
            }
            return optionalvalue(&found->second);
        }

        optionalvalue at(size_t index) {
            check_type(type, value_type::List);
            return optionalvalue(&val.list->at(index));
        }

        const optionalvalue at(size_t index) const {
            check_type(type, value_type::List);
            return optionalvalue(&val.list->at(index));
        }

        bool has(const key_t& k) const;

        size_t size() const noexcept;

        size_t length() const noexcept {
            return size();
        }
        inline bool empty() const noexcept {
            return size() == 0;
        }

        inline bool isString() const noexcept {
            return type == value_type::String;
        }
        inline bool isObject() const noexcept {
            return type == value_type::Object;
        }
        inline bool isList() const noexcept {
            return type == value_type::List;
        }
        inline bool isInteger() const noexcept {
            return type == value_type::Integer;
        }
        inline bool isNumber() const noexcept {
            return type == value_type::Number;
        }
    };

    inline bool is_numeric(const value& val) {
        return val.isInteger() || val.isNumber();
    }
}

namespace dv {
    inline const std::string& type_name(const value& value) {
        return type_name(value.getType());
    }

    inline value object() {
        return std::make_shared<objects::Object>();
    }

    inline value object(std::initializer_list<pair> pairs) {
        return std::make_shared<objects::Object>(std::move(pairs));
    }

    inline value list() {
        return std::make_shared<objects::List>();
    }

    inline value list(std::initializer_list<value> values) {
        return std::make_shared<objects::List>(std::move(values));
    }

    template<typename T> inline bool get_to_int(value* ptr, T& dst) {
        if (ptr) {
            dst = ptr->asInteger();
            return true;
        }
        return false;
    }
    template<typename T> inline bool get_to_num(value* ptr, T& dst) {
        if (ptr) {
            dst = ptr->asNumber();
            return true;
        }
        return false;
    }
    inline bool optionalvalue::get(std::string& dst) const {
        if (ptr) {
            dst = ptr->asString();
            return true;
        }
        return false;
    }

    inline bool optionalvalue::get(bool& dst) const {
        if (ptr) {
            dst = ptr->asBoolean();
            return true;
        }
        return false;
    }

    inline bool optionalvalue::get(char& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(short& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(int& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(long& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(long long& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(unsigned char& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(unsigned short& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(unsigned int& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(unsigned long& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(unsigned long long& dst) const {
        return get_to_int(ptr, dst);
    }
    inline bool optionalvalue::get(float& dst) const {
        return get_to_num(ptr, dst);
    }
    inline bool optionalvalue::get(double& dst) const {
        return get_to_num(ptr, dst);
    }
}

std::ostream& operator<<(std::ostream& stream, const dv::value& value);
