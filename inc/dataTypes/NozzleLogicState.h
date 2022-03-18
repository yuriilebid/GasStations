#ifndef PTS2_0_NOZZLELOGICSTATE_H
#define PTS2_0_NOZZLELOGICSTATE_H

#include <memory>
#include "UniversalAuthorize.h"
#include "TechCmd.h"

enum class ScenarioLastState : int {
    IDLE,
    NOZZLE_UP,
    AUTHORIZED,
    TEN_PERCENT,
    NINETY_PERCENT,
    HUNDRED_PERCENT,
    CLOSE
};

class NozzleLogicState {
private:
    int nozzle{};
    int requestedVolume{};
    int price{};
    LogicType logic;
    ScenarioLastState lastState;
public:
    NozzleLogicState() {};
    NozzleLogicState(int _nozzle, int _volume, int _price, ScenarioLastState state, LogicType _logic);
    NozzleLogicState(std::unique_ptr<UniversalAuthorize> cmd, ScenarioLastState state, LogicType _type);

    ScenarioLastState getLastState() const;
    int getNozzle() const;
    int getVolume() const;
    int getPrice() const;

    LogicType getLogicType() const;
    void setLogicType(LogicType type);
    void setVolume(int val);
    void updateLastState();

    void setType(ScenarioLastState _type);
};

#endif //PTS2_0_NOZZLELOGICSTATE_H
