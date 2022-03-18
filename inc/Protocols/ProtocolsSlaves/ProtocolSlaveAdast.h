#ifndef PTS2_0_PROTOCOLSLAVEADAST_H
#define PTS2_0_PROTOCOLSLAVEADAST_H

#include "ProtocolSlave.h"

class ProtocolSlaveAdast : public ProtocolSlave {
private:
public:
    std::unique_ptr<UniversalCmd> parseCmd(const std::vector<unsigned char>& cmd) override;
    std::vector<unsigned char> preparePacketVec(std::shared_ptr<UniversalState> cmd, int addr) override;
    int getMinPacketSize() override {return 7;};
    bool checkCheckSum(const std::vector<unsigned char>& cmd) override { return {}; };
    std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) override { return {}; };
    std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) override { return {}; };
    /// Specific for JsonPts
    std::string preparePacketStr(std::unique_ptr<UniversalTech> cmd) override { return {}; };
    std::string preparePacketStr(std::shared_ptr<UniversalState> cmd, const std::string &userName) override { return {}; };
    std::string gatherResponses(const std::vector<std::string> &responses) override { return {}; };
    std::vector<std::vector<unsigned char>> splitPackets(const std::vector<unsigned char>& srcPacket) override { return {}; };
};


#endif //PTS2_0_PROTOCOLSLAVEADAST_H
