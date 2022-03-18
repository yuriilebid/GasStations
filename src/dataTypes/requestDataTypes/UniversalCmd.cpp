#include "UniversalCmd.h"
#include <iostream>

int UniversalCmd::getId() {
    return Id;
}

CmdType UniversalCmd::getCmdType() {
    return cmdType;
}

void UniversalCmd::setCmdType(CmdType _type) {
    cmdType = _type;
}

void UniversalCmd::setId(int _id) {
    Id = _id;
}

void UniversalCmd::setControlUser(const std::string& user) {
    controlUser = user;
}

std::string UniversalCmd::getControlUser() {
    return controlUser;
}

void UniversalCmd::setDbId(int vId) {
    dbId = vId;
}

int UniversalCmd::getDbId() {
    return dbId;
}

UniversalCmd::UniversalCmd(int _columnId, CmdType _type):
    Id(_columnId), cmdType(_type) {
}


