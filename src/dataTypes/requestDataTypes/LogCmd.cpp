#include "LogCmd.h"

void LogCmd::setLogLevel(columnId _id, bool traceStatus) {
    columnTraceLogs[_id] = traceStatus;
}

std::map<columnId, bool> LogCmd::getLogLevel() {
    return columnTraceLogs;
}
