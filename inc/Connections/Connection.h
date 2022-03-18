#ifndef PTS2_0_CONNECTION_H
#define PTS2_0_CONNECTION_H

#include <string>
#include <vector>

#include "logger.h"

#include <nlohmann/json.hpp>

enum class ConnectionType: uint8_t {
    TCP,
    SERIAL,
    UNKNOWN
};

NLOHMANN_JSON_SERIALIZE_ENUM( ConnectionType, {
    {ConnectionType::TCP, "tcp"},
    {ConnectionType::SERIAL, "serial"},
})

class Connection {
private:
    int port;
    unsigned int failedReading;
    bool connectionInited = false;
    bool traceLogsState = false;
    bool uniqueTraces = false;
protected:
    std::string address;
    int maxTimeReadPacketMillis = 200;
public:
    bool getConnectionInited() const;
    bool getTraceLogsState() const;
    bool getUniqueTraces() const;
    void setTraceLogsState(bool st);
    void setUniqueTraces(bool st);
    void setConnectionInited(bool status);
    void incrementFailedReading();
    void clearFailedReading();

    virtual bool connectInit() = 0;
    virtual void closeConnection() = 0;
    virtual std::vector<unsigned char>
    transmitByStartEnd(const std::vector<unsigned char> &packet, const std::vector<unsigned char> &start,
                       const std::vector<unsigned char> &end, int sizeIndex, bool logTrace,
                       int additionalSize) = 0;
    virtual std::vector<unsigned char> readPacketBySize(bool logTrace, int sizeIndex) = 0;
    virtual std::vector<unsigned char> readPacketByByte(bool logTrace, int expectedSize) = 0;
    virtual std::vector<unsigned char> transmit(const std::vector<unsigned char> &packet, bool logTrace) = 0;
    virtual void writePacket(const std::vector<unsigned char> &packet, bool logTrace) = 0;
    virtual std::vector<unsigned char> readPacket(bool logTrace) = 0;

    template <ConnectionType type>
    static std::unique_ptr<Connection> CreateConnection(const nlohmann::json& cfg_obj);
};



#endif //PTS2_0_CONNECTION_H
