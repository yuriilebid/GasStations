#include "Protocols/ProtocolsSlaves/ProtocolSlave.h"

bool ProtocolSlave::checkPacketAppropriation(const std::vector<unsigned char>& cmd) {
    if(cmd.size() < getMinPacketSize()) {
        return false;
    }
    auto crcCorrect = checkCheckSum(cmd);

    if(!crcCorrect) {
//        LogPrintf(spdlog::level::warn, "Fiscal CRC is incorrect!");
        return false;
    } else {
        return true;
    }
}
