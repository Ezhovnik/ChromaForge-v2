#ifndef CODERS_JSON_H_
#define CODERS_JSON_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>

#include "commons.h"
#include "../typedefs.h"
#include "binary_json.h"
#include "../data/dynamic.h"

namespace json {
    std::unique_ptr<dynamic::Map> parse(const std::string& filename, const std::string& source);
    std::unique_ptr<dynamic::Map> parse(const std::string& source);

    std::string stringify(const dynamic::Map* obj, bool nice, const std::string& indent);
}


#endif // CODERS_JSON_H_
