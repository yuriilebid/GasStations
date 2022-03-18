#include "Line.h"

#include "TcpConnection.h"
#include "serialConnection.h"
#include "ProtocolMasterAdast.h"
#include "ProtocolMasterUnipump.h"
#include "ProtocolMasterShelf.h"


Line::Line() {
}

std::unique_ptr<Connection> Line::createConnection(const nlohmann::json &conn_cfg, ConnectionType conn_type) {
    switch (conn_type) {
        case ConnectionType::TCP:
            return std::make_unique<TcpConnection>(conn_cfg);
        case ConnectionType::SERIAL:
            return std::make_unique<SerialConnection>(conn_cfg);
        default:
            throw std::invalid_argument(fmt::format("Unknown connection type: {}", conn_type));
    }
}

Line::~Line() {
}

