#ifndef DATA_DYNAMIC_FWD_H_
#define DATA_DYNAMIC_FWD_H_

#include <memory>
#include <variant>
#include <string>
#include <functional>

#include <typedefs.h>

namespace dynamic {
    class Map;
    class List;

    using Map_sptr = std::shared_ptr<Map>;
    using List_sptr = std::shared_ptr<List>;

    struct none {};

    inline constexpr none NONE = {};

    using Value = std::variant<
        none,
        Map_sptr,
        List_sptr,
        std::string,
        number_t,
        bool,
        integer_t
    >;

    using to_string_func = std::function<std::string(const Value&)>;
}

#endif // DATA_DYNAMIC_FWD_H_
