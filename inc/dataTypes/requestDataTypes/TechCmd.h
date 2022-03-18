#ifndef PTS2_0_TECHCMD_H
#define PTS2_0_TECHCMD_H

#include "UniversalCmd.h"

enum class LogicType;

class TechCmd : public UniversalCmd {
private:
    int nozzle{};
    LogicType logicType{};
public:
    TechCmd(int id, int _nozzle, LogicType type);
    TechCmd(): UniversalCmd(0, CmdType::CHANGE_SCENARIOS) {
        setCmdType(CmdType::CHANGE_SCENARIOS);
    };
    int getNozzle();
    LogicType getLogicType();
    TechCmd& setNozzle(int _nozzle);
    TechCmd& setLogicType(LogicType _type);
};


#endif //PTS2_0_TECHCMD_H
