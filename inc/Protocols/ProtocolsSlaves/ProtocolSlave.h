#ifndef PTS2_0_PROTOCOLSLAVE_H
#define PTS2_0_PROTOCOLSLAVE_H

#include <memory>
#include <vector>

#include "UniversalCmd.h"
#include "UniversalState.h"
#include "ColumnConfig.h"
#include "UniversalTech.h"

enum class FiscalType {
    MARIA,
    TECHNO
};

class ProtocolSlave {
protected:
public:
    ColumnConfig columnCfg{};
    uint8_t currentAddr{};
    uint8_t currentPacketId{};
    uint8_t sizeIndex{};
    FiscalType type = FiscalType::TECHNO;

    ProtocolSlave() = default;

    bool checkPacketAppropriation(const std::vector<unsigned char>& cmd);

    virtual std::vector<unsigned char> traceResponse(std::vector<unsigned char> packet) = 0;
    virtual std::vector<unsigned char> traceRequests(std::vector<unsigned char> packet) = 0;
    virtual std::unique_ptr<UniversalCmd> parseCmd(const std::vector<unsigned char>& cmd) = 0;
    virtual bool checkCheckSum(const std::vector<unsigned char>& cmd) = 0;
    virtual std::vector<unsigned char> preparePacketVec(std::shared_ptr<UniversalState> cmd, int addr) = 0;
    virtual int getMinPacketSize() = 0;
    /// Specififc to Protocol Json Slave
    virtual std::string preparePacketStr(std::shared_ptr<UniversalState> cmd, const std::string &userName) = 0;
    virtual std::string preparePacketStr(std::unique_ptr<UniversalTech> cmd) = 0;
    virtual std::string gatherResponses(const std::vector<std::string> &responses) = 0;
    virtual std::vector<std::vector<unsigned char>> splitPackets(const std::vector<unsigned char>& srcPacket) = 0;
};


#endif //PTS2_0_PROTOCOLSLAVE_H
