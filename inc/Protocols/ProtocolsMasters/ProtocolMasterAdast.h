#ifndef PTS2_0_PROTOCOLMASTERADAST_H
#define PTS2_0_PROTOCOLMASTERADAST_H

#include "UniversalState.h"
#include "ProtocolMaster.h"
#include "UniversalAuthorize.h"
#include "UniversalClose.h"
#include "UniversalTotal.h"
#include "UniversalTotals.h"
#include "UniversalAuthorizeConfirmation.h"
#include <map>

class ProtocolMasterAdast : public ProtocolMaster {
private:
    int cmdIndex = 4;
    std::map<int, int> pumpIds;
    std::map<int, int> lastPriceAuthorize;

    std::map<int, std::vector<unsigned char>> lastSentPacket;
    std::map<int, std::vector<unsigned char>> lastReceivedPacket;
    std::map<int, std::vector<unsigned char>> lastReceived51Packet;

    std::vector<unsigned char> getLastPacket(int addr) override;
    /// Creating packet to TRK
    static std::vector<unsigned char> statusRequest(std::unique_ptr<UniversalCmd> lastTransaction, int addr);
    std::vector<unsigned char> runCommand(std::unique_ptr<UniversalAuthorize> cmd, int addr);
    static std::vector<unsigned char> stopCommand(std::unique_ptr<UniversalCmd> cmd, int addr);
    std::vector<unsigned char> closeCommand(std::unique_ptr<UniversalClose> cmd, int addr);
    static std::vector<unsigned char> totalCommand(std::unique_ptr<UniversalTotal> cmd, int addr);
    static std::vector<unsigned char> setPriceRequest(std::unique_ptr<UniversalAuthorize> cmd, int addr);
    /// Parsing packets from TRK
    std::shared_ptr<UniversalState> parseAmountInfo(const std::vector<unsigned char>& cmd);
    std::shared_ptr<UniversalState> parseEndDispensingInfo(const std::vector<unsigned char>& cmd);
    std::shared_ptr<UniversalState> parseTotalsInfo(const std::vector<unsigned char> &cmd, int id);
    std::shared_ptr<UniversalAuthorizeConfirmation> resolveActiveKeyboard(const std::vector<unsigned char>& cmd);
    static std::shared_ptr<UniversalState> parseHangedFreeInfo(const std::vector<unsigned char>& cmd);
    static std::shared_ptr<UniversalState> parseRemovedInfo(const std::vector<unsigned char>& cmd);
    static unsigned int calc_crc(const std::vector<unsigned char> &cmd);
public:
    ProtocolMasterAdast() = default;
    std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> preparePacket(std::unique_ptr<UniversalCmd> cmd, int addr) override;
    std::shared_ptr<UniversalState>
    parseResponse(const std::vector<unsigned char> &cmd, int expectedPumpAddr, int pumpId) override;
    bool checkPacketAppropriation(const std::vector<unsigned char> &cmd) override;
    int getAddrOfPacket(const std::vector<unsigned char>& cmd) override { return cmd.at(2); };
    bool checkCheckSum(const std::vector<unsigned char>& cmd) override;
    int getMinPacketSize() override {return 5;}
    std::vector<unsigned char> getStartPacketVec() override {return {0x16, 0x16};};
    int getLengthIndex() override {return 4;};
    int getAdditionalSize() override {return 7;};
};


#endif //PTS2_0_PROTOCOLMASTERADAST_H
