#ifndef PTS2_0_PROTOCOLMASTERUNIPUMP_H
#define PTS2_0_PROTOCOLMASTERUNIPUMP_H

#include "ProtocolMaster.h"
#include <vector>
#include "UniversalState.h"
#include "UniversalAuthorize.h"
#include "UniversalClose.h"
#include "UniversalTotal.h"
#include <memory>
#include <map>

class ProtocolMasterUnipump : public ProtocolMaster {
private:
    unsigned char DLE = 0x10;
    unsigned char STX = 0x02;
    unsigned char ETX = 0x03;
    unsigned short cmdIndex = 3;
    std::map<int, std::vector<unsigned char>> lastSentPacket;
    std::map<int, std::vector<unsigned char>> lastReceivedPacket;
    std::map<int, std::pair<int, int>> transactionHolderAddrId;

    std::vector<unsigned char> statusRequest(std::unique_ptr<UniversalCmd> cmd, int addr);
    std::vector<unsigned char> runCommand(std::unique_ptr<UniversalAuthorize> cmd, int addr);
    std::vector<unsigned char> stopCommand(std::unique_ptr<UniversalCmd> cmd, int addr);
    std::vector<unsigned char> setPriceRequest(std::unique_ptr<UniversalCmd> cmd, int addr);
    std::vector<unsigned char> totalCommand(std::unique_ptr<UniversalTotal> cmd, int addr);
    std::vector<unsigned char> closeCommand(std::unique_ptr<UniversalClose> cmd, int addr);
    static unsigned int calc_crc(const std::vector<unsigned char> &cmd);
public:
    std::vector<unsigned char> getLastPacket(int addr) override {};
    std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) override;
    std::vector<unsigned char> preparePacket(std::unique_ptr<UniversalCmd> cmd, int addr) override;
    std::shared_ptr<UniversalState>
    parseResponse(const std::vector<unsigned char> &cmd, int expectedPumpAddr, int pumpId) override;
    bool checkPacketAppropriation(const std::vector<unsigned char> &cmd) override;
    int getAddrOfPacket(const std::vector<unsigned char>& cmd) override {
        return cmd.at(2) - 0x30;
    }
    std::vector<unsigned char> getStartPacketVec() override {return {0x16, 0x16};};
    int getLengthIndex() override {return 4;};
    bool checkCheckSum(const std::vector<unsigned char>& cmd) override;
    int getMinPacketSize() override {return 8;};
    int getAdditionalSize() override {return 7;};
};


#endif //PTS2_0_PROTOCOLMASTERUNIPUMP_H
