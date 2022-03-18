#ifndef PTS2_0_SCENARIO_H
#define PTS2_0_SCENARIO_H

#include "UniversalCmd.h"
#include "UniversalState.h"
#include "UniversalAmount.h"
#include "ProtocolSlave.h"
#include "UniversalClose.h"
#include "nozzleLogicState.h"
#include "TechCmd.h"

#include <mutex>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>

using ColumnId = int;

class Scenario {
private:
    LogicType type {};
    int timeActivityCommandWaiting {};
    std::mutex stateMx;
    std::map<ColumnId, NozzleLogicState> states;
    std::shared_ptr<UniversalState> lastStateResponse;
    std::map<ColumnId, std::chrono::steady_clock::time_point> timeStamps;
public:
    std::shared_ptr<UniversalState> processCmd(std::unique_ptr<UniversalCmd> cmd);
    std::shared_ptr<UniversalState> operateSleepState(std::unique_ptr<UniversalCmd> cmd);
    void setTime(int colId);
    std::chrono::steady_clock::time_point getTimeNow();
    std::chrono::steady_clock::time_point getTime(int colId);
    NozzleLogicState getState(ColumnId id);

    Scenario() = default;
    explicit Scenario(nlohmann::json jsobObj);
    void addState(ColumnId id, NozzleLogicState columnState);
    void updateState(ColumnId id, ScenarioLastState state, int newVolume);
    std::unique_ptr<TechCmd> changeLogicType(std::unique_ptr<TechCmd> cmd);

    bool checkActivityTime(int colId);

    /// SLEEP proccesting commands
    std::shared_ptr<UniversalState> operateSleepIdle(std::unique_ptr<UniversalCmd> cmd);
    std::shared_ptr<UniversalState> operateSleppNozzleUp(std::unique_ptr<UniversalCmd> cmd);
    std::shared_ptr<UniversalState> operateSleppAuthorized(std::unique_ptr<UniversalCmd> cmd);
    std::shared_ptr<UniversalState> operateSleppTenPercent(std::unique_ptr<UniversalCmd> cmd);
    std::shared_ptr<UniversalState> operateSleppNinetyPercent(std::unique_ptr<UniversalCmd> cmd);
    std::shared_ptr<UniversalState> operateSleppHundredPercent(std::unique_ptr<UniversalCmd> cmd);
    std::shared_ptr<UniversalState> operateSleppClose(std::unique_ptr<UniversalCmd> cmd);
};

#endif //PTS2_0_SCENARIO_H
