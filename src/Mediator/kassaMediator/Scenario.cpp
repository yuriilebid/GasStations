#include "Scenario.h"
#include "UniversalAmount.h"
#include "UniversalClose.h"
#include "UniversalTotals.h"
#include <memory>
#include "logger.h"

std::shared_ptr<UniversalState> Scenario::operateSleepIdle(std::unique_ptr<UniversalCmd> cmd) {
    ColumnId colId = cmd->getId();
    auto cmdType = cmd->getCmdType();
    auto lastState = states.at(colId);

    if(cmdType == CmdType::WRITE_VOLUME) {
        auto authiorizeCmd = std::make_unique<UniversalAuthorize>(*dynamic_cast<UniversalAuthorize *>(cmd.release()));
        int nozzle = authiorizeCmd->getNozzle();
        NozzleLogicState cmdAuthorized = NozzleLogicState(std::move(authiorizeCmd), ScenarioLastState::AUTHORIZED, LogicType::SLEEP);

        LogPrintf(spdlog::level::info, "Last state |IDLE| Cmd from |FISCAL| |WRITE_VOLUME|");
        addState(colId, cmdAuthorized);
        lastStateResponse = std::make_shared<UniversalState>(colId, nozzle, StateType::REMOVED_WAITING, 0);
        return lastStateResponse;
    } else {
        return std::make_shared<UniversalState>(colId, lastState.getNozzle(), StateType::REMOVED_WAITING, 0);
    }
}

std::shared_ptr<UniversalState> Scenario::operateSleppNozzleUp(std::unique_ptr<UniversalCmd> cmd) {
    ColumnId colId = cmd->getId();
    auto cmdType = cmd->getCmdType();

    if(cmdType == CmdType::WRITE_VOLUME) {
        auto authorizeCmd = std::make_unique<UniversalAuthorize>(*dynamic_cast<UniversalAuthorize*>(cmd.release()));
        int nozzle = authorizeCmd->getNozzle();

        NozzleLogicState cmdAuthorized = NozzleLogicState(std::move(authorizeCmd), ScenarioLastState::AUTHORIZED, LogicType::SLEEP);
        LogPrintf(spdlog::level::info, "Last state |NOZZLE_UP| Cmd from |FISCAL| |WRITE_VOLUME|");
        std::shared_ptr<UniversalState> curState;
        addState(colId, cmdAuthorized);
        curState->setId(colId);
        curState->setType(StateType::REMOVED_WAITING);
        curState->setNozzle(nozzle);
        lastStateResponse = curState;
        return curState;
    } else {
        LogPrintf(spdlog::level::info, "Last state |NOZZLE_UP| Cmd from |FISCAL| |ELSE|");
        return lastStateResponse;
    }
}

std::shared_ptr<UniversalState> Scenario::operateSleppAuthorized(std::unique_ptr<UniversalCmd> cmd) {
    auto colId = cmd->getId();
    auto cmdType = cmd->getCmdType();
    auto lastState = getState(colId);

    if(cmdType == CmdType::STATUS_REQUEST) {
        LogPrintf(spdlog::level::info, "Last state |AUTHORIZED| Cmd from |FISCAL| |STATUS_REQUEST|");
        int nozzle = lastState.getNozzle();
        int tenVolume = static_cast<int>(static_cast<float>(lastState.getVolume()) / 10.0);
        int price = lastState.getPrice();
        int sum = (tenVolume / 1000 * price);

        lastStateResponse = std::make_shared<UniversalAmount>(colId, nozzle, 0, tenVolume, price, 0, StateType::FUEL_SUPPLY, sum);
        updateState(colId, ScenarioLastState::TEN_PERCENT, tenVolume);
        return lastStateResponse;
    } else {
        LogPrintf(spdlog::level::info, "Last state |AUTHORIZED| Cmd from |FISCAL| |ELSE|");
        return lastStateResponse;
    }
}

std::shared_ptr<UniversalState> Scenario::operateSleppTenPercent(std::unique_ptr<UniversalCmd> cmd) {
    auto colId = cmd->getId();
    auto cmdType = cmd->getCmdType();
    auto lastState = getState(colId);

    if (cmdType == CmdType::STATUS_REQUEST) {
        int nozzle = lastState.getNozzle();
        int ninetyVolume = lastState.getVolume() * 9;
        int price = lastState.getPrice();
        int sum = (ninetyVolume / 1000 * price);

        LogPrintf(spdlog::level::info, "Last state |TEN_PERCENT| Cmd from |FISCAL| |STATUS_REQUEST|");
        updateState(colId, ScenarioLastState::NINETY_PERCENT, ninetyVolume);
        lastStateResponse = std::make_shared<UniversalAmount>(colId, nozzle, 0, ninetyVolume, price, 0, StateType::FUEL_SUPPLY, sum);
        return lastStateResponse;
    } else {
        LogPrintf(spdlog::level::info, "Last state |TEN_PERCENT| Cmd from |FISCAL| |ELSE|");
        return lastStateResponse;
    }
}

std::shared_ptr<UniversalState> Scenario::operateSleppNinetyPercent(std::unique_ptr<UniversalCmd> cmd) {
    auto colId = cmd->getId();
    auto cmdType = cmd->getCmdType();
    auto lastState = getState(colId);

    if(cmdType == CmdType::STATUS_REQUEST) {
        int nozzle = lastState.getNozzle();
        int ninetyVolume = static_cast<int>(static_cast<float>(lastState.getVolume()) * 10.0) / 9;
        int price = lastState.getPrice();
        int sum = (ninetyVolume / 1000 * price);

        LogPrintf(spdlog::level::info, "Last state |NINETY_PERCENT| Cmd from |FISCAL| |STATUS_REQUEST|");
        updateState(colId, ScenarioLastState::HUNDRED_PERCENT, ninetyVolume);
        lastStateResponse = std::make_shared<UniversalAmount>(colId, nozzle, 0, ninetyVolume, price, 0, StateType::SUPPLY_DONE, sum);
        return lastStateResponse;
    } else {
        LogPrintf(spdlog::level::info, "Last state |NINETY_PERCENT| Cmd from |FISCAL| |ELSE|");
        return lastStateResponse;
    }
}

std::shared_ptr<UniversalState> Scenario::operateSleppHundredPercent(std::unique_ptr<UniversalCmd> cmd) {
    auto colId = cmd->getId();
    auto cmdType = cmd->getCmdType();
    auto lastState = getState(colId);

    if(cmdType == CmdType::STOP_COMMAND) {
        LogPrintf(spdlog::level::info, "Last state |HUNDRED_PERCENT| Cmd from |FISCAL| |CLOSE_REPORT|");
        states[colId] = NozzleLogicState(0, 0, 0, ScenarioLastState::IDLE, LogicType::IDLE);
        states.at(colId).setLogicType(LogicType::IDLE);
        return std::make_shared<UniversalAmount>(colId, 0, 0, 0, 0, 0, StateType::SUPPLY_STOPPED, 0);
     } else {
         LogPrintf(spdlog::level::info, "Last state |HUNDRED_PERCENT| Cmd from |FISCAL| |ELSE|");
         return lastStateResponse;
     }
}

std::shared_ptr<UniversalState> Scenario::operateSleppClose(std::unique_ptr<UniversalCmd> cmd) {
    auto colId = cmd->getId();
    auto cmdType = cmd->getCmdType();
    auto lastState = getState(colId);

    if(cmdType == CmdType::STOP_COMMAND) {
        LogPrintf(spdlog::level::info, "Last state |CLOSE| Cmd from |FISCAL| |CLOSE_REPORT|");
        std::shared_ptr<UniversalState> endState;
        updateState(colId, ScenarioLastState::IDLE, 0);
        endState->setId(colId);
        endState->setNozzle(lastState.getNozzle());
        endState->setType(StateType::SUPPLY_STOPPED);
        lastStateResponse = endState;
        states.at(colId).setLogicType(LogicType::IDLE);
        return endState;
    } else {
        LogPrintf(spdlog::level::info, "Last state |CLOSE| Cmd from |FISCAL| |ELSE|");
        return lastStateResponse;
    }
}

std::shared_ptr<UniversalState> Scenario::operateSleepState(std::unique_ptr<UniversalCmd> cmd) {
    ColumnId columnId = cmd->getId();
    const auto lastState = getState(columnId);


    switch(lastState.getLastState()) {
        case ScenarioLastState::IDLE:
            if(!checkActivityTime(columnId)) {
                states.at(columnId).setLogicType(LogicType::IDLE);
            }
            LogPrintf(spdlog::level::info, "|Stepper| waiting |WRITE_VOLUME|");
            return operateSleepIdle(std::move(cmd));
        case ScenarioLastState::NOZZLE_UP:
            LogPrintf(spdlog::level::info, "|Stepper| Past state |NOZZLE_UP|");
            return operateSleppNozzleUp(std::move(cmd));
        case ScenarioLastState::AUTHORIZED:
            LogPrintf(spdlog::level::info, "|Stepper| Past state |AUTHORIZED|");
            return operateSleppAuthorized(std::move(cmd));
        case ScenarioLastState::TEN_PERCENT:
            LogPrintf(spdlog::level::info, "|Stepper| Past state |TEN_PERCENT|");
            return operateSleppTenPercent(std::move(cmd));
        case ScenarioLastState::NINETY_PERCENT:
            LogPrintf(spdlog::level::info, "|Stepper| Past state |NINETY_PERCENT|");
            return operateSleppNinetyPercent(std::move(cmd));
        case ScenarioLastState::HUNDRED_PERCENT:
            LogPrintf(spdlog::level::info, "|Stepper| Past state |HUNDRED_PERCENT|");
            return operateSleppHundredPercent(std::move(cmd));
        case ScenarioLastState::CLOSE:
            LogPrintf(spdlog::level::info, "|Stepper| Past state |CLOSE|");
            return operateSleppClose(std::move(cmd));
    }
    return std::make_shared<UniversalState>(columnId, lastState.getNozzle(), StateType::REMOVED_WAITING, 0);
}

Scenario::Scenario(const nlohmann::json jsobObj) {
    try {
        for(auto& line : jsobObj.at("lines")) {
            for(auto& col : line.at("columns")) {
                int pumpId = col.at("Id").get<int>();
                LogPrintf(spdlog::level::info, "Setting |IDLE| state for |Fiscal| on pump |{}|", pumpId);
                NozzleLogicState idleState(0, 0, 0, ScenarioLastState::IDLE, LogicType::IDLE);
                states.insert(std::pair<int, NozzleLogicState>(pumpId, idleState));
            }
        }
        for(auto& kassa : jsobObj.at("fiscals")) {
            timeActivityCommandWaiting = kassa.at("simulation_waiting");
        }
    } catch(const nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "Parsing config in Fiscals: {}", e.what());
    }
}

std::chrono::steady_clock::time_point Scenario::getTimeNow() {
    return std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point Scenario::getTime(int colId) {
    if(timeStamps.find(colId) == timeStamps.end()) {
        return getTimeNow();
    } else {
        return timeStamps.at(colId);
    }
}

void Scenario::setTime(int colId) {
    timeStamps[colId] = getTimeNow();
}

std::shared_ptr<UniversalState> Scenario::processCmd(std::unique_ptr<UniversalCmd> cmd) {
    auto columnId = cmd->getId();
    if(states.find(columnId) == states.end()) {
//        LogPrintf(spdlog::level::warn, "|FISCAl| Asks for pump |{}| witch was not configured", columnId);
        UniversalState idleState;
        idleState.setId(columnId);
        idleState.setType(StateType::HANGED_FREE);
        return std::make_shared<UniversalState>(idleState);
    }
    auto logicType = states.at(columnId).getLogicType();

    switch(logicType) {
        case LogicType::IDLE: {
            if(cmd->getCmdType() == CmdType::GET_TOTAL) {
                UniversalTotals totalStatus;
                totalStatus.setId(columnId);
                totalStatus.setType(StateType::TOTAL_RESPONSE);

                return std::make_shared<UniversalTotals>(totalStatus);
            } else {
                UniversalState idleState;
                idleState.setId(columnId);
                idleState.setType(StateType::HANGED_FREE);
                return std::make_shared<UniversalState>(idleState);
            }
        }
        case LogicType::SLEEP:
            return operateSleepState(std::move(cmd));
        case LogicType::CONTROL:
            return {};
        default:
            break;
    }
    return {};
}

NozzleLogicState Scenario::getState(ColumnId id) {
    std::lock_guard<std::mutex> lock(stateMx);
    auto copyState = states.at(id);
    return copyState;
}

void Scenario::addState(ColumnId id, NozzleLogicState columnState) {
    std::lock_guard<std::mutex> lock(stateMx);
    setTime(id);
    states.at(id) = columnState;
}

void Scenario::updateState(ColumnId id, ScenarioLastState state, int newVolume = 0) {
    std::lock_guard<std::mutex> lock(stateMx);
    states.at(id).setType(state);
    if(newVolume != 0) {
        states.at(id).setVolume(newVolume);
    }
}

bool Scenario::checkActivityTime(int colId) {
    auto nowTime = getTimeNow();
    auto updateTime = getTime(colId);
    auto timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(nowTime - updateTime).count();

    if(timeElapsed > timeActivityCommandWaiting) {
        if(timeElapsed > timeActivityCommandWaiting and timeElapsed < timeActivityCommandWaiting + 2) {
            LogPrintf(spdlog::level::warn, "|FISCAL| Did not send appropriate commnad during the last {} seconds. Changing scenario to |IDLE|", timeElapsed);
        }
        return false;
    }
    return true;

}

std::unique_ptr<TechCmd> Scenario::changeLogicType(std::unique_ptr<TechCmd> cmd) {
    auto colId = cmd->getId();

    states[colId] = NozzleLogicState(cmd->getNozzle(), 0, 0, ScenarioLastState::IDLE,  cmd->getLogicType());
    states.at(colId).setLogicType(cmd->getLogicType());
    if(cmd->getLogicType() == LogicType::SLEEP) {
        setTime(colId);
    }
    LogPrintf(spdlog::level::info, "|PTS| Changed scenario type for pump |{}|", colId);
    return cmd;
}
