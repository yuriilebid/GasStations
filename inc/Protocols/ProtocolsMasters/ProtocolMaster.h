#ifndef PTS2_0_PROTOCOLMASTER_H
#define PTS2_0_PROTOCOLMASTER_H

#include <UniversalCmd.h>
#include <UniversalState.h>
#include <memory>
#include <vector>

class ProtocolMaster {
private:
public:
    virtual std::vector<unsigned char> getLastPacket(int addr) = 0;
    virtual std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) = 0;
    virtual std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) = 0;
    virtual std::vector<unsigned char> preparePacket(std::unique_ptr<UniversalCmd> cmd, int addr) = 0;
    virtual std::shared_ptr<UniversalState>
    parseResponse(const std::vector<unsigned char> &cmd, int expectedPumpAddr, int pumpId) = 0;
    virtual bool checkPacketAppropriation(const std::vector<unsigned char> &cmd) = 0;
    virtual int getAddrOfPacket(const std::vector<unsigned char>& cmd) = 0;
    virtual int getMinPacketSize() = 0;
    virtual bool checkCheckSum(const std::vector<unsigned char>& cmd) = 0;
    virtual std::vector<unsigned char> getStartPacketVec() = 0;
    virtual int getLengthIndex() = 0;
    virtual int getAdditionalSize() = 0;

    double getNumberValueOfHexArray(std::vector<uint8_t> arr, int posStart, int posEnd);
};


#endif //PTS2_0_PROTOCOLMASTER_H
