#ifndef PTS2_0_TCPCONNECTION_H
#define PTS2_0_TCPCONNECTION_H

#include "Connection.h"
#include <nlohmann/json.hpp>

#define	TIMEOUT		10
#define	MAX_SOCKETS	3

class TcpConnection : public Connection {
private:
    std::vector<unsigned char> lastPacket, lastWrote;
    std::string address;
    int port;
    int fd{};
public:
    explicit TcpConnection(const nlohmann::json& cfgObj);
    bool connectInit() override;
    void closeConnection() override;
    std::vector<unsigned char>
    transmitByStartEnd(const std::vector<unsigned char> &packet, const std::vector<unsigned char> &start,
                       const std::vector<unsigned char> &end, int sizeIndex, bool logTrace,
                       int additionalSize) override;
    std::vector<unsigned char> readPacketBySize(bool logTrace, int sizeIndex) override;
    std::vector<unsigned char> readPacketByByte(bool logTrace, int expectedSize) override;
    std::vector<unsigned char> transmit(const std::vector<unsigned char> &packet, bool logTrace) override;
    void writePacket(const std::vector<unsigned char> &packet, bool logTrace) override;
    std::vector<unsigned char> readPacket(bool logTrace = false) override;
};


#endif //PTS2_0_TCPCONNECTION_H
