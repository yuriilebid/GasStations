#ifndef PTS2_0_COLUMNLINE_H
#define PTS2_0_COLUMNLINE_H

#include <nlohmann/json.hpp>

#include "Line.h"
#include "ProtocolMaster.h"

using columnId = int;

class ColumnLine : public Line, public std::enable_shared_from_this<ColumnLine> {
private:
    bool working = false;
    bool traceLogsState = false;
    std::unique_ptr<ProtocolMaster> proto;
    std::map<columnId, bool> traceStates;
    StateMap states;
    safeQueue<std::unique_ptr<UniversalCmd>> columnCmds;
    safeQueue<std::unique_ptr<UniversalCmd>> technicalCmds;

    explicit ColumnLine(const nlohmann::json& cfgObj);
public:
    void poll() override;

    [[maybe_unused]] void setWorkingState(bool state);
    bool getWorkingState() const;
    void setConfig(const std::map<int, FuelProduct>& cfgProducts);
    std::shared_ptr<UniversalState> getState(ColumnId id);
    void addCmd(std::unique_ptr<UniversalCmd> cmd);
    std::future<void> start() override;

    [[maybe_unused]] void addTechCmd(std::unique_ptr<UniversalCmd> cmd);
    void createProtocol(ColumnProtocolType _type);
    void changeLog(int _id, bool logTraceStatus);
    static std::shared_ptr<ColumnLine> create(const nlohmann::json& cfgObj);
    bool checkPacket(const std::vector<unsigned char>& cmd);

    ColumnConfig lineCfg;

    void processCmd();

    void processStatus(const ColumnId columnId, Addr columnAddr);
};


#endif //PTS2_0_COLUMNLINE_H
