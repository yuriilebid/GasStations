#include "Connections/Connection.h"
#include "serialConnection.h"
#include "TcpConnection.h"

void Connection::incrementFailedReading() {
    failedReading++;
    if(failedReading > 100) {
//        if(fd > 0) {
            closeConnection();
//        }
        connectionInited = false;
    }
}

void Connection::clearFailedReading() {
    failedReading = 0;
}

template<>
std::unique_ptr<Connection> Connection::CreateConnection<ConnectionType::SERIAL>(const nlohmann::json &cfg_obj) {
    return std::make_unique<SerialConnection>(cfg_obj);
}

bool Connection::getConnectionInited() const {
    return connectionInited;
}

bool Connection::getTraceLogsState() const {
    return traceLogsState;
}

void Connection::setTraceLogsState(bool st) {
    traceLogsState = st;
}

bool Connection::getUniqueTraces() const {
    return uniqueTraces;
}

void Connection::setUniqueTraces(bool st) {
    uniqueTraces = st;
}

void Connection::setConnectionInited(bool status) {
    connectionInited = status;
}

template<>
std::unique_ptr<Connection> Connection::CreateConnection<ConnectionType::TCP>(const nlohmann::json &cfg_obj) {
    return std::make_unique<TcpConnection>(cfg_obj);
}