#pragma once

#include <nlohmann/json.hpp>


struct PG_DisaStartingStats { //8 unsigned chars
    unsigned char hp;
    unsigned char sp;
    unsigned char attack;
    unsigned char defence;
    unsigned char intelligence;
    unsigned char speed;
    unsigned char hit;
    unsigned char resistence;

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

/**
* @brief Simmilar to DisaStats but smaller in size
*/
struct PG_DisaBaseStats { //16 unsigned chars
    unsigned short hp;
    unsigned short sp;
    unsigned short attack;
    unsigned short defence;
    unsigned short intelligence;
    unsigned short speed;
    unsigned short hit;
    unsigned short resistence;

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};

struct PG_DisaStats { //32 unsigned chars
    unsigned int hp;
    unsigned int sp;
    unsigned int attack;
    unsigned int defence;
    unsigned int intelligence;
    unsigned int speed;
    unsigned int hit;
    unsigned int resistence;

    void operator=(const PG_DisaStartingStats& other) {
        hp = other.hp;
        sp = other.sp;
        attack = other.attack;
        defence = other.defence;
        intelligence = other.intelligence;
        speed = other.speed;
        hit = other.hit;
        resistence = other.resistence;
    }

    void operator=(const PG_DisaBaseStats& other) {
        hp = other.hp;
        sp = other.sp;
        attack = other.attack;
        defence = other.defence;
        intelligence = other.intelligence;
        speed = other.speed;
        hit = other.hit;
        resistence = other.resistence;
    }

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);
};