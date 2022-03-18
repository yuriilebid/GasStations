#include "UniversalState.h"

int UniversalState::getId() const {
    return columnId;
}

int UniversalState::getNozzle() const {
    return activeNozzle;
}

int UniversalState::getDbId() const {
    return dbId;
}

StateType UniversalState::getType() const {
    return type;
}

void UniversalState::setId(int id) {
    columnId = id;
}

void UniversalState::setNozzle(int nozzle) {
    activeNozzle = nozzle;
}

void UniversalState::setType(StateType _type) {
    type = _type;
}

void UniversalState::setDbId(int idFromDb) {
    dbId = idFromDb;
}

std::string UniversalState::getControlUser() {
    return controlUser;
}

void UniversalState::setControlUser(const std::string& _user) {
    controlUser = _user;
}

UniversalState::UniversalState(int _columnId, int _activeNozzle, StateType _type, int _dbId) :
columnId(_columnId), activeNozzle(_activeNozzle), type(_type), dbId(_dbId) {}

UniversalState UniversalState::getIdleState(ColumnId id) {
    return UniversalState(id, 0, StateType::HANGED_FREE, 0);
}