#ifndef PTS2_0_SERIALCONNECTION_H
#define PTS2_0_SERIALCONNECTION_H

#include "Connection.h"
#include "nlohmann/json.hpp"

class SerialConnection : public Connection {
private:
public:
    bool connectInit() override { return {}; };
    void closeConnection() override {}
    std::vector<unsigned char>
    transmitByStartEnd(const std::vector<unsigned char> &packet, const std::vector<unsigned char> &start,
                       const std::vector<unsigned char> &end, int sizeIndex, bool logTrace,
                       int additionalSize) override {};
    explicit SerialConnection(const nlohmann::json& cfgObj);
    std::vector<unsigned char> readPacketBySize(bool logTrace, int sizeIndex) override { return {}; };
    std::vector<unsigned char> readPacketByByte(bool logTrace, int expectedSize) override { return {}; };
    std::vector<unsigned char> transmit(const std::vector<unsigned char> &packet, bool logTrace) override;
    void writePacket(const std::vector<unsigned char> &packet, bool logTrace) override {};
    std::vector<unsigned char> readPacket(bool logTrace) override { return {}; };
};







#endif //PTS2_0_SERIALCONNECTION_H
