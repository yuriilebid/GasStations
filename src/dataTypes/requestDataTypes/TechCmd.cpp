#include "TechCmd.h"

TechCmd::TechCmd(int id, int _nozzle, LogicType type) : UniversalCmd(id, CmdType::CHANGE_SCENARIOS) {
    nozzle = _nozzle;
    logicType = type;
}

int TechCmd::getNozzle() {
    return nozzle;
}

TechCmd& TechCmd::setNozzle(int _nozzle) {
    nozzle = _nozzle;
    return *this;
}

TechCmd& TechCmd::setLogicType(LogicType _type) {
    logicType = _type;
    return *this;
}

LogicType TechCmd::getLogicType() {
    return logicType;
}
