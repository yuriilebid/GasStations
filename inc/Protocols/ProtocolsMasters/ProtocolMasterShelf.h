#ifndef PTS2_0_PROTOCOLMASTERSHELF_H
#define PTS2_0_PROTOCOLMASTERSHELF_H

#include "ProtocolMaster.h"
#include "UniversalAuthorize.h"
#include "UniversalTotal.h"
#include "UniversalClose.h"
#include "UniversalAmount.h"
#include "UniversalTotals.h"
#include "UniversalAuthorizeConfirmation.h"
#include "logger.h"
#include <map>

class ProtocolMasterShelf : public ProtocolMaster {
private:
    std::map<int, int> currentPrices;
    std::map<unsigned char, unsigned char> packetIndexCounter;
    const int commandByte = 4;
    unsigned int getCounterRequest(unsigned char _address);
    int calcrc(std::vector<unsigned char> ptr, int count);
    int resolveActiveNozzle(unsigned char nozzleStateByte);
    StateType resolveTrkState(unsigned char nozzleStateByte);
    std::vector<unsigned char> totalCommand(std::unique_ptr<UniversalTotal> cmd, int addr);

    std::vector<unsigned char> statusRequest(std::unique_ptr<UniversalCmd> cmd, int addr);
    std::vector<unsigned char> runCommand(std::unique_ptr<UniversalAuthorize> cmd, int addr);
    std::vector<unsigned char> stopCommand(std::unique_ptr<UniversalCmd> cmd, int addr);
    std::vector<unsigned char> setPriceRequest(std::unique_ptr<UniversalAuthorize> cmd, int addr);
    std::shared_ptr<UniversalState> resolveStopStatus(const std::vector<unsigned char> &cmd);
    std::shared_ptr<UniversalAmount> resolveAmountInfo(const std::vector<unsigned char>& cmd);
    std::shared_ptr<UniversalAmount> resolveMadeAmount(const std::vector<unsigned char>& cmd);
    std::shared_ptr<UniversalTotals> resolveTotal(const std::vector<unsigned char>& cmd);
    std::shared_ptr<UniversalAmount> resolveCurrentAmount(const std::vector<unsigned char>& cmd);
    std::shared_ptr<UniversalAuthorizeConfirmation> resolveActiveKeyboard (const std::vector<unsigned char>& cmd, bool ready);
public:
    ProtocolMasterShelf() = default;
    std::vector<unsigned char> getLastPacket(int addr) override {};
    std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> preparePacket(std::unique_ptr<UniversalCmd> cmd, int addr) override;
    std::shared_ptr<UniversalState>
    parseResponse(const std::vector<unsigned char> &cmd, int expectedPumpAddr, int pumpId) override;
    bool checkPacketAppropriation(const std::vector<unsigned char> &cmd) override;
    int getAddrOfPacket(const std::vector<unsigned char>& cmd) override { return {}; };
    int getMinPacketSize() override {return 8;};
    bool checkCheckSum(const std::vector<unsigned char>& cmd) override { return {}; };
    std::vector<unsigned char> getStartPacketVec() override {};
    int getLengthIndex() override {};
    int getAdditionalSize() override {};
};


#endif //PTS2_0_PROTOCOLMASTERSHELF_H
