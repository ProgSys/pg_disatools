#include "PG_DisaStats.h"
#include <Disa/fileformats/PG_JsonUtil.h>

using namespace PG_JsonUtil;

nlohmann::json PG_DisaStartingStats::toJson() const {
    return {
        {"hp", hp},
        {"sp", sp},
        {"attack", attack},
        {"defence", defence},
        {"intelligence", intelligence},
        {"speed", speed},
        {"hit", hit},
        {"resistence", resistence}
    };
}

void PG_DisaStartingStats::fromJson(const nlohmann::json& j) {
    readFromJson(j, "hp", hp);
    readFromJson(j, "sp", sp);
    readFromJson(j, "attack", attack);
    readFromJson(j, "defence", defence);
    readFromJson(j, "intelligence", intelligence);
    readFromJson(j, "speed", speed);
    readFromJson(j, "hit", hit);
    readFromJson(j, "resistence", resistence);
}

nlohmann::json PG_DisaBaseStats::toJson() const {
    return {
        {"hp", hp},
        {"sp", sp},
        {"attack", attack},
        {"defence", defence},
        {"intelligence", intelligence},
        {"speed", speed},
        {"hit", hit},
        {"resistence", resistence}
    };
}

void PG_DisaBaseStats::fromJson(const nlohmann::json& j) {
    readFromJson(j, "hp", hp);
    readFromJson(j, "sp", sp);
    readFromJson(j, "attack", attack);
    readFromJson(j, "defence", defence);
    readFromJson(j, "intelligence", intelligence);
    readFromJson(j, "speed", speed);
    readFromJson(j, "hit", hit);
    readFromJson(j, "resistence", resistence);
}

nlohmann::json PG_DisaStats::toJson() const {
    return {
        {"hp", hp},
        {"sp", sp},
        {"attack", attack},
        {"defence", defence},
        {"intelligence", intelligence},
        {"speed", speed},
        {"hit", hit},
        {"resistence", resistence}
    };
}

void PG_DisaStats::fromJson(const nlohmann::json& j) {
    readFromJson(j, "hp", hp);
    readFromJson(j, "sp", sp);
    readFromJson(j, "attack", attack);
    readFromJson(j, "defence", defence);
    readFromJson(j, "intelligence", intelligence);
    readFromJson(j, "speed", speed);
    readFromJson(j, "hit", hit);
    readFromJson(j, "resistence", resistence);
}