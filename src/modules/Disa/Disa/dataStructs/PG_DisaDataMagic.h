#pragma once
#include <Disa/dataStructs/PG_DisaStats.h>

/**
* @brief From Magic.dat
*/
struct PG_DisaDataMagic { //size 86
    unsigned short id;
    unsigned short level;
    unsigned short pow;
    unsigned short sp;

    std::array<char, 33> name; //shiftJis encoded
    std::array<char, 113> description; //shiftJis encoded

    std::array<unsigned char, 5> effectPrams;
    unsigned char wt;
    unsigned char e;
    unsigned char st;
    unsigned char wt2;
    unsigned char rg;
    unsigned char pat;
    unsigned char up;
    unsigned char dn;
    std::array<unsigned char, 5> effect;

    std::string getName() const;
    void setName(const std::string& name);

    std::string getDescription() const;
    void setDescription(const std::string& str);


    void fromJson(const nlohmann::json& j);
    std::string toString() const;
};