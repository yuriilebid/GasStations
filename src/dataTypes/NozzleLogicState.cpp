#include "nozzleLogicState.h"

NozzleLogicState::NozzleLogicState(int _nozzle, int _volume, int _price, ScenarioLastState state, LogicType _logic) :
        nozzle(_nozzle), requestedVolume(_volume), price(_price), lastState(state), logic(_logic){}

int NozzleLogicState::getNozzle() const {
    return nozzle;
}

void NozzleLogicState::setType(ScenarioLastState _type) {
    lastState = _type;
}

int NozzleLogicState::getPrice() const {
    return price;
}

int NozzleLogicState::getVolume() const {
    return requestedVolume;
}

ScenarioLastState NozzleLogicState::getLastState() const {
    return lastState;
}

LogicType NozzleLogicState::getLogicType() const {
    return logic;
}

void NozzleLogicState::setLogicType(LogicType type) {
    logic = type;
}

void NozzleLogicState::setVolume(int val) {
    requestedVolume = val;
}

void NozzleLogicState::updateLastState() {
    switch(lastState) {
        case ScenarioLastState::IDLE:
            lastState = ScenarioLastState::NOZZLE_UP;
            break;
        case ScenarioLastState::NOZZLE_UP:
            lastState = ScenarioLastState::AUTHORIZED;
            break;
        case ScenarioLastState::AUTHORIZED:
            lastState = ScenarioLastState::TEN_PERCENT;
            break;
        case ScenarioLastState::TEN_PERCENT:
            lastState = ScenarioLastState::NINETY_PERCENT;
            break;
        case ScenarioLastState::NINETY_PERCENT:
            lastState = ScenarioLastState::HUNDRED_PERCENT;
            break;
        case ScenarioLastState::HUNDRED_PERCENT:
            lastState = ScenarioLastState::CLOSE;
            break;
        case ScenarioLastState::CLOSE:
            lastState = ScenarioLastState::IDLE;
            break;
    }
}

NozzleLogicState::NozzleLogicState(std::unique_ptr<UniversalAuthorize> cmd, ScenarioLastState state, LogicType _type) :
nozzle(cmd->getNozzle()), requestedVolume(cmd->getVolume()), price(cmd->getPrice()), lastState(state), logic(_type) {}