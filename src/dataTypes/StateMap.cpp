#include "dataTypes/StateMap.h"
#include "UniversalAmount.h"
#include <iostream>
#include <utility>
#include "logger.h"

std::chrono::steady_clock::time_point StateMap::getTimeNow() {
    return std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point StateMap::getTime(int colId) {
    if(timeStamps.find(colId) == timeStamps.end()) {
        return getTimeNow();
    } else {
        return timeStamps.at(colId);
    }
}

void StateMap::setTime(int colId) {
    timeStamps[colId] = getTimeNow();
}

void StateMap::setUserUpdateValidity(bool stateUpdate, int _pumpId) {
    userUpdateAvailable[_pumpId] = stateUpdate;
}

bool StateMap::getUserUpdateValidity(int _pumpId) {
    if(userUpdateAvailable.find(_pumpId) != userUpdateAvailable.end()) {
        return userUpdateAvailable.at(_pumpId);
    } else {
        return true;
    }
}

void StateMap::setDataValidityTime(int timeSeconds) {
    dataValidityTime = timeSeconds;
}

bool StateMap::checkStateActuality(int colId) {
    auto lastState = getTime(colId);
    auto timeNow = getTimeNow();
    auto timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(timeNow - lastState).count();

    if(timeElapsed > dataValidityTime) {
        if(timeElapsed > dataValidityTime and timeElapsed < dataValidityTime + 2) {
            LogPrintf(spdlog::level::warn, "|TRK| Have not been responding for the last {} seconds", timeElapsed);
        }
        return false;
    }
    return true;
}

std::shared_ptr<UniversalState> StateMap::getState(ColumnId id) {
    std::lock_guard<std::mutex> lock(stateMx);

    if(states.count(id)) {
        std::shared_ptr<UniversalState> stateRes = states.at(id);

        if(checkStateActuality(id)) {
            if (stateRes->getType() == StateType::TOTAL_RESPONSE) {
                LogPrintf(spdlog::level::info, "Got TOTAL response");
                states[id] = std::make_shared<UniversalState>(id, 0, StateType::NOT_SERVED, 0);
            }
        } else {
            stateRes->setType(StateType::NOT_SERVED);
        }
        return stateRes;
    } else {
        std::shared_ptr<UniversalState> stateRes = std::make_shared<UniversalState>(id, 0, StateType::NOT_SERVED, 0);
        return stateRes;
    }
}

const std::string getStateString(StateType type) {
    switch(type) {
        case StateType::BUSY:
            return "BUSY";
        case StateType::NOT_SERVED:
            return "NOT_SERVED";
        case StateType::HANGED_FREE:
            return "HANGED_FREE";
        case StateType::REMOVED_WAITING:
            return "REMOVED_WAITING";
        case StateType::REMOVED_FREE:
            return "REMOVED_FREE";
        case StateType::WAITING_APPROPRIATE_PISTOL:
            return "WAITING_APPROPRIATE_PISTOL";
        case StateType::REMOVED_MANUAL_DOSING:
            return "REMOVED_MANUAL_DOSING";
        case StateType::AUTHORIZE_REGISTERED:
            return "AUTHORIZE_REGISTERED";
        case StateType::TEST_INDICATORS:
            return "TEST_INDICATORS";
        case StateType::CUTOFF_FUEL_ACTIVE:
            return "CUTOFF_FUEL_ACTIVE";
        case StateType::FUEL_SUPPLY:
            return "FUEL_SUPPLY";
        case StateType::SLOWING_FUEL_ACTIVE:
            return "SLOWING_FUEL_ACTIVE";
        case StateType::FUEL_ENGINE_ACTIVE:
            return "FUEL_ENGINE_ACTIVE";
        case StateType::SUPPLY_STOPPED_SC:
            return "SUPPLY_STOPPED_SC";
        case StateType::SUPPLY_STOPPED:
            return "SUPPLY_STOPPED";
        case StateType::SUPPLY_STOPPED_NO_PULSE:
            return "SUPPLY_STOPPED_NO_PULSE";
        case StateType::SUPPLY_STOPPED_NEST:
            return "SUPPLY_STOPPED_NEST";
        case StateType::SUPPLY_STOPPED_NO_POWER:
            return "SUPPLY_STOPPED_NO_POWER";
        case StateType::SUPPLY_DONE:
            return "SUPPLY_DONE";
        case StateType::SUPPLY_DONE_REPORT_OPEN:
            return "SUPPLY_DONE_REPORT_OPEN";
        case StateType::WAITING_CONFIRM_EERROR:
            return "WAITING_CONFIRM_EERROR";
        case StateType::SUPPLY_DONE_REPORT_CLOSE:
            return "SUPPLY_DONE_REPORT_CLOSE";
        case StateType::LOCK_STATUS:
            return "LOCK_STATUS";
        case StateType::TOTAL_RESPONSE:
            return "TOTAL_RESPONSE";
        case StateType::TRANSACTION_ClOSED:
            return "TRANSACTION_ClOSED";
        case StateType::TRANSACTION_UNClOSED:
            return "TRANSACTION_UNClOSED";
        case StateType::ERROR:
            return "ERROR";
    }
}

void StateMap::LogInfo(ColumnId id, const std::shared_ptr<UniversalState>& newState) {
    auto newType = newState->getType();
    auto oldType = states.at(id)->getType();

    switch(newType) {
        case StateType::BUSY:
            if(oldType != StateType::BUSY) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |BUSY|", id, getStateString(oldType));
            }
            break;
        case StateType::NOT_SERVED:
            if(oldType != StateType::NOT_SERVED) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |BUSY|", id, getStateString(oldType));
            }
            break;
        case StateType::HANGED_FREE:
            if(oldType != StateType::HANGED_FREE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |HANGED_FREE|", id, getStateString(oldType));
            }
            break;
        case StateType::REMOVED_WAITING:
            if(oldType != StateType::REMOVED_WAITING) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |REMOVED_WAITING|", id, getStateString(oldType));
            }
            break;
        case StateType::REMOVED_FREE:
            if(oldType != StateType::REMOVED_FREE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |REMOVED_FREE|", id, getStateString(oldType));
            }
            break;
        case StateType::WAITING_APPROPRIATE_PISTOL:
            if(oldType != StateType::WAITING_APPROPRIATE_PISTOL) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |WAITING_APPROPRIATE_PISTOL|", id, getStateString(oldType));
            }
            break;
        case StateType::REMOVED_MANUAL_DOSING:
            if(oldType != StateType::REMOVED_MANUAL_DOSING) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |REMOVED_MANUAL_DOSING|", id, getStateString(oldType));
            }
            break;
        case StateType::AUTHORIZE_REGISTERED:
            if(oldType != StateType::AUTHORIZE_REGISTERED) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |AUTHORIZE_REGISTERED|", id, getStateString(oldType));
            }
            break;
        case StateType::TEST_INDICATORS:
            if(oldType != StateType::TEST_INDICATORS) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |TEST_INDICATORS|", id, getStateString(oldType));
            }
            break;
        case StateType::CUTOFF_FUEL_ACTIVE:
            if(oldType != StateType::CUTOFF_FUEL_ACTIVE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |CUTOFF_FUEL_ACTIVE|", id, getStateString(oldType));
            }
            break;
        case StateType::FUEL_SUPPLY:
            if(oldType != StateType::FUEL_SUPPLY) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |FUEL_SUPPLY|", id, getStateString(oldType));
            } else {
                auto voluneInfo = std::static_pointer_cast<UniversalAmount>(newState);
                LogPrintf(spdlog::level::info, "|COLUMN||{}||FUEL_SUPPLY| - |{}| ml.", id, voluneInfo->getVolume());
            }
            break;
        case StateType::SLOWING_FUEL_ACTIVE:
            if(oldType != StateType::SLOWING_FUEL_ACTIVE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SLOWING_FUEL_ACTIVE|", id, getStateString(oldType));
            }
            break;
        case StateType::FUEL_ENGINE_ACTIVE:
            if(oldType != StateType::FUEL_ENGINE_ACTIVE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |FUEL_ENGINE_ACTIVE|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_STOPPED_SC:
            if(oldType != StateType::SUPPLY_STOPPED_SC) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_STOPPED_SC|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_STOPPED:
            if(oldType != StateType::SUPPLY_STOPPED) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_STOPPED|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_STOPPED_NO_PULSE:
            if(oldType != StateType::SUPPLY_STOPPED_NO_PULSE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_STOPPED_NO_PULSE|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_STOPPED_NEST:
            if(oldType != StateType::SUPPLY_STOPPED_NEST) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_STOPPED_NEST|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_STOPPED_NO_POWER:
            if(oldType != StateType::SUPPLY_STOPPED_NO_POWER) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_STOPPED_NO_POWER|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_DONE:
            if(oldType != StateType::SUPPLY_DONE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_DONE|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_DONE_REPORT_OPEN:
            if(oldType != StateType::SUPPLY_DONE_REPORT_OPEN) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_DONE_REPORT_OPEN|", id, getStateString(oldType));
            }
            break;
        case StateType::WAITING_CONFIRM_EERROR:
            if(oldType != StateType::WAITING_CONFIRM_EERROR) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |WAITING_CONFIRM_EERROR|", id, getStateString(oldType));
            }
            break;
        case StateType::SUPPLY_DONE_REPORT_CLOSE:
            if(oldType != StateType::SUPPLY_DONE_REPORT_CLOSE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |SUPPLY_DONE_REPORT_CLOSE|", id, getStateString(oldType));
            }
            break;
        case StateType::LOCK_STATUS:
            if(oldType != StateType::LOCK_STATUS) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |LOCK_STATUS|", id, getStateString(oldType));
            }
            break;
        case StateType::TOTAL_RESPONSE:
            if(oldType != StateType::TOTAL_RESPONSE) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |TOTAL_RESPONSE|", id, getStateString(oldType));
            }
            break;
        case StateType::TRANSACTION_ClOSED:
            if(oldType != StateType::TRANSACTION_ClOSED) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |TRANSACTION_ClOSED|", id, getStateString(oldType));
            }
            break;
        case StateType::TRANSACTION_UNClOSED:
            if(oldType != StateType::TRANSACTION_UNClOSED) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |TRANSACTION_UNClOSED|", id, getStateString(oldType));
            }
            break;
        case StateType::ERROR:
            if(oldType != StateType::ERROR) {
                LogPrintf(spdlog::level::info, "|COLUMN||{}| Old State: |{}| -> New State: |ERROR|", id, getStateString(oldType));
            }
            break;
    }
}

void StateMap::addState(ColumnId id, const std::shared_ptr<UniversalState>& newState) {
    std::lock_guard<std::mutex> lock(stateMx);
    if (newState and newState->getId() != 0) {
        if(states.find(id) != states.end()) {
            LogInfo(id, newState);
            if(states.at(id)->getType() != StateType::TOTAL_RESPONSE) {
                if(!getUserUpdateValidity(id)) {
                    newState->setControlUser(states.at(id)->getControlUser());
                } else {
                    newState->setControlUser("none");
                }
                states[id] = newState;
            }
        } else {
            LogPrintf(spdlog::level::info, "|COLUMN||{}| Init State: |{}|", id, getStateString(newState->getType()));
            if(!getUserUpdateValidity(id)) {
                newState->setControlUser(states.at(id)->getControlUser());
            } else {
                newState->setControlUser("none");
            }
            states[id] = newState;
        }
        setTime(id);
    }
}
