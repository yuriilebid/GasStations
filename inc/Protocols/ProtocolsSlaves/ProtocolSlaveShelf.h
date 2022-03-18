#ifndef PTS2_0_PROTOCOLSLAVESHELF_H
#define PTS2_0_PROTOCOLSLAVESHELF_H

#include "ProtocolSlave.h"
#include "UniversalSetPrice.h"
#include "UniversalAuthorize.h"
#include "UniversalAmount.h"
#include "UniversalTotals.h"

enum shelfCommandTypes {
    StatusRequest = 0x01,
    ReadPrice = 0x02,
    WritePrice = 0x03,
    AmountInfo = 0x04,
    WriteVolume = 0x05,
    StopCommand = 0x0C,
    WriteMoney = 0x09,
    WriteFullTank = 0x06,
    TotalCountersRequest = 0x15,
    CurrentCountersRequest = 0x16
};

class ProtocolSlaveShelf : public ProtocolSlave {
private:
    std::map<int, std::vector<unsigned char>> lastSentPacket;
    std::map<int, std::vector<unsigned char>> lastReceivedPacket;

    int cmdIndex = 4;
public:
    ProtocolSlaveShelf() {sizeIndex = 3;};
    std::unique_ptr<UniversalCmd> parseCmd(const std::vector<unsigned char>& cmd) override;
    std::vector<unsigned char> preparePacketVec(std::shared_ptr<UniversalState> cmd, int addr) override;
    int getMinPacketSize() override {return 7;};
    bool checkCheckSum(const std::vector<unsigned char>& cmd) override;

    std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) override;
    /// Specific for ProtocolSlaveShelf
    std::vector<unsigned char> responseHangedFree(const std::shared_ptr<UniversalState>& data);
    std::vector<unsigned char> responseRemovedNozzle(const std::shared_ptr<UniversalState>& data);
    std::vector<unsigned char> responseAmountInfo(const std::shared_ptr<UniversalAmount>& data);
    std::vector<unsigned char> responseSupplyDone(const std::shared_ptr<UniversalAmount>& data);
    std::vector<unsigned char> responseConfirmation(const std::shared_ptr<UniversalState> &data);
    std::vector<unsigned char> responseClosedTransaciton(const std::shared_ptr<UniversalAmount>& data);
    std::vector<unsigned char> responseTotaInfo(const std::shared_ptr<UniversalTotals>& data);
    /// Specific for JsonPts
    std::string preparePacketStr(std::unique_ptr<UniversalTech> cmd) override { return {}; };
    std::string preparePacketStr(std::shared_ptr<UniversalState> cmd, const std::string &userName) override { return {}; };
    std::string gatherResponses(const std::vector<std::string> &responses) override { return {}; };
    std::vector<std::vector<unsigned char>> splitPackets(const std::vector<unsigned char>& srcPacket) override { return {}; };

    static std::unique_ptr<UniversalAuthorize> resolveWriteVolumeRequest(const std::vector<unsigned char> &cmd);
    UniversalSetPrice resolveSetPriceRequest(const std::vector<unsigned char> &cmd);
};


#endif //PTS2_0_PROTOCOLSLAVESHELF_H
