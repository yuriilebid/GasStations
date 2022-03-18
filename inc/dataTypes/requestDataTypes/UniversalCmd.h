#ifndef PTS2_0_UNIVERSALCMD_H
#define PTS2_0_UNIVERSALCMD_H

#include <string>

enum class LogicType: int {
    IDLE,
    SLEEP,
    CONTROL
};

enum class CmdType {
    STATUS_REQUEST,
    WRITE_VOLUME,
    STOP_COMMAND,
    READ_PRICE,
    WRITE_PRICE,
    GET_TOTAL,
    PAUSE_COMMAND,
    WRITE_MONEY,
    CLOSE_REPORT,
    CHANGE_LOG,
    CHANGE_SCENARIOS,
    GRADES_CONFIG,
    PUMP_CONFIG,
    GET_BATTERY,
    GET_SD,
    CONFIRMATION
};

class UniversalCmd {
private:
    int Id{};
    int dbId{};
    CmdType cmdType{};
    std::string controlUser;
public:
    UniversalCmd() : Id(0), cmdType(CmdType::STATUS_REQUEST) {};
    UniversalCmd(int _columnId, CmdType _type);
    virtual ~UniversalCmd() = default;
    int getId();
    CmdType getCmdType();
    void setId(int _id);
    void setControlUser(const std::string& user);
    std::string getControlUser();
    void setDbId(int vId);
    int getDbId();
    void setCmdType(CmdType _type);
};


#endif //PTS2_0_UNIVERSALCMD_H
