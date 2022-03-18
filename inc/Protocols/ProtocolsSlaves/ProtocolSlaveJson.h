#ifndef PTS2_0_PROTOCOLSLAVEJSON_H
#define PTS2_0_PROTOCOLSLAVEJSON_H

#include "ProtocolSlave.h"
#include <nlohmann/json.hpp>
#include "UniversalAmount.h"
#include "UniversalTotals.h"
#include "UniversalGradesConfig.h"
#include "UniversalPumpsConfig.h"

enum class jsonCmdType {
    STATUS,
    AUTHORIZE,
    CLOSE,
    STOP,
    SIMULATE,
    SD_INFO,
    BATTERY,
    FUEL_CONFIG,
    PUMP_CONFIG,
    SET_PRICE,
    TOTALS,
    SET_DATE,
    RESTART
};

NLOHMANN_JSON_SERIALIZE_ENUM( jsonCmdType, {
    {jsonCmdType::STATUS, "PumpGetStatus"},
    {jsonCmdType::TOTALS, "PumpGetTotals"},
    {jsonCmdType::AUTHORIZE, "PumpAuthorize"},
    {jsonCmdType::CLOSE, "PumpCloseTransaction"},
    {jsonCmdType::STOP, "PumpStop"},
    {jsonCmdType::SIMULATE, "PumpSetSimulateFillingOnDartInput"},
    {jsonCmdType::BATTERY, "GetBatteryVoltage"},
    {jsonCmdType::SD_INFO, "GetSdInformation"},
    {jsonCmdType::FUEL_CONFIG, "GetFuelGradesConfiguration"},
    {jsonCmdType::PUMP_CONFIG, "GetPumpNozzlesConfiguration"},
    {jsonCmdType::SET_PRICE, "SetFuelGradesConfiguration"},
    {jsonCmdType::SET_DATE, "SetDateTime"},
    {jsonCmdType::RESTART, "Restart"}
})

class ProtocolSlaveJson : public ProtocolSlave {
private:
    int packetId;
    std::unordered_map<int, UniversalAmount> lastDispense;
    std::vector<unsigned char> lastResponseStatus;
    std::vector<unsigned char> lastResponse;
    std::vector<unsigned char> lastRequestStatus;
    std::vector<unsigned char> lastRequest;

    /// Pump cmd control
    std::string resolveHangedFreeResponse(const std::shared_ptr<UniversalState>& cmd);
    std::string resolveRemovedResponse(const std::shared_ptr<UniversalState>& cmd);
    std::string resolveFuelSupply(const std::shared_ptr<UniversalAmount>& cmd);
    std::string resolveSupplyDone(const std::shared_ptr<UniversalAmount>& cmd);
    std::string resolveTotals(const std::shared_ptr<UniversalTotals>& cmd);
    std::string resolveNotServerResponse(const std::shared_ptr<UniversalState>& cmd);
    std::string resolveConfirmation();
    std::string resolveAuthorizeResponse(const std::shared_ptr<UniversalState>& cmd);
    std::string resolveError(const std::string &errMsg);
    /// Tech cmd control
    std::string resolveConfigurationResponse(std::unique_ptr<UniversalGradesConfig> cfg);
    static std::string resolveLogsResponse(std::unique_ptr<UniversalTech> cmd);
    std::string resolvePumpConfigurationResponse(std::unique_ptr<UniversalPumpsConfig> data);
    std::string resolveSdResponse();
    std::string resolveBatteryResponse();
public:
    std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) override;
    std::unique_ptr<UniversalCmd> parseCmd(const std::vector<unsigned char>& cmd) override;
    std::vector<unsigned char> preparePacketVec(std::shared_ptr<UniversalState> cmd, int addr) override;
    int getMinPacketSize() override {return 7;};
    bool checkCheckSum(const std::vector<unsigned char>& cmd) override { return false; };

    [[maybe_unused]] static std::vector<UniversalCmd> parseMultipleCmd(const std::vector<unsigned char>& cmd);

    std::string preparePacketStr(std::shared_ptr<UniversalState> cmd, const std::string &userName) override;
    std::string preparePacketStr(std::unique_ptr<UniversalTech> cmd) override;
    std::vector<std::vector<unsigned char>> splitPackets(const std::vector<unsigned char>& srcPacket) override;
    std::string gatherResponses(const std::vector<std::string> &responses) override;
};


#endif //PTS2_0_PROTOCOLSLAVEJSON_H
