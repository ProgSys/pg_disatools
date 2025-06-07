#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <Util/Debug/PG_DebugOut.h>

namespace PG_JsonUtil {
    static inline void readFromJson(auto& j, const std::string& field, auto& target, unsigned int max = 128u) {
        if (auto it = j.find(field); it != j.end()) {
            it->get_to(target);
            if constexpr (std::is_same_v<decltype(target), std::string>)
                target.resize(std::min(target.size(), max));
        }
    }

    static inline bool readFromJsonMandatory(auto& j, const std::string& field, auto& target, unsigned int max = 128u) {
        readFromJson(j, field, target, max);

        if constexpr (std::is_same_v<decltype(target), std::string&>) {
            if (target.empty()) {
                OUT_ERROR("'Json is missing the mandatory field '" << field << "'!");
                return false;
            }
        }

        return true;
    }

}