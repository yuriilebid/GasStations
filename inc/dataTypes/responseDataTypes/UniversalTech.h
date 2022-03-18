#ifndef PTS2_0_UNIVERSALTECH_H
#define PTS2_0_UNIVERSALTECH_H

#include "UniversalState.h"

enum class techCmdType : int {
    CONFIGURATION,
    PUMP_CONFIGURATION,
    SD_INFO,
    BATTERY,
    LOGS,
    SET_PRICE,
    SCENARIO
};

class UniversalTech {
private:
    techCmdType type {};
public:
    UniversalTech() = default;
    UniversalTech(techCmdType _type): type(_type) {};
    virtual ~UniversalTech();
    techCmdType getType();
    void setType(techCmdType _type);
};


#endif //PTS2_0_UNIVERSALTECH_H
