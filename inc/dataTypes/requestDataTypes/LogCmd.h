#ifndef PTS2_0_LOGCMD_H
#define PTS2_0_LOGCMD_H

#include "TechCmd.h"
#include <map>

using columnId = int;

class LogCmd : public TechCmd {
private:
    std::map<columnId, bool> columnTraceLogs;
public:
    void setLogLevel(columnId _id, bool traceStatus);
    std::map<columnId, bool> getLogLevel();
};


#endif //PTS2_0_LOGCMD_H
