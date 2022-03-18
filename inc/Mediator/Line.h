#ifndef PTS2_0_LINE_H
#define PTS2_0_LINE_H

#include <queue>
#include <map>
#include <future>
#include <mutex>

#include "mxMap.h"
#include "mxQueue.h"
#include <nlohmann/json.hpp>
#include <utility>

#include "Connection.h"
#include "UniversalCmd.h"
#include "UniversalState.h"
#include "ColumnConfig.h"
#include "StateMap.h"

using ColumnId  = int;

enum class ColumnProtocolType: uint8_t {
    UNIPUMP,
    ADAST,
    SHELF,
    UNKNOWN,
};

// map TaskState values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(ColumnProtocolType, {
    {ColumnProtocolType::UNKNOWN, nullptr},
    {ColumnProtocolType::UNIPUMP, "unipump"},
    {ColumnProtocolType::ADAST, "adast"},
    {ColumnProtocolType::SHELF, "shelf"},
})

class Line {
protected:
    std::unique_ptr<Connection> connect{};
public:
    explicit Line();
    ~Line();
    virtual std::future<void> start() = 0;
    virtual void poll() = 0;

    static std::unique_ptr<Connection> createConnection(const nlohmann::json &conn_cfg, ConnectionType conn_type);
};


#endif //PTS2_0_LINE_H
