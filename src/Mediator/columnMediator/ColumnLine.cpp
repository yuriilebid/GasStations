#include "ColumnLine.h"
#include "ProtocolMasterShelf.h"
#include "ProtocolMasterUnipump.h"
#include "ProtocolMasterAdast.h"

void ColumnLine::createProtocol(ColumnProtocolType _type) {
    switch(_type) {
        case ColumnProtocolType::UNIPUMP:
            proto = std::make_unique<ProtocolMasterUnipump>();
            break;
        case ColumnProtocolType::ADAST:
            proto = std::make_unique<ProtocolMasterAdast>();
            break;
        case ColumnProtocolType::SHELF:
            proto = std::make_unique<ProtocolMasterShelf>();
            break;
        case ColumnProtocolType::UNKNOWN:
            LogPrintf(spdlog::level::err, "Unknown protocol");
            break;
    }
}

void ColumnLine::changeLog(columnId _id, bool logTraceStatus) {
    traceStates.at(_id) = logTraceStatus;
}

std::shared_ptr<ColumnLine> ColumnLine::create(const nlohmann::json& cfgObj) {
    return std::shared_ptr<ColumnLine>(new ColumnLine(cfgObj));
}

void ColumnLine::setConfig(const std::map<int, FuelProduct>& cfgProducts) {
    LogPrintf(spdlog::level::info, "Size of cfgProducts: {}", cfgProducts.size());
    lineCfg.productIdProduct = cfgProducts;
}

ColumnLine::ColumnLine(const nlohmann::json& cfgObj) {
    try {
        auto& conn_cfg = cfgObj.at("communication");
        auto conn_type = conn_cfg.at("transportation").get<ConnectionType>();
        auto protocol_type = cfgObj.at("protocol").get<ColumnProtocolType>();

        for(auto& columnCfg : cfgObj.at("columns")) {
            bool traceEnabled = columnCfg.at("trace");
            int colId = columnCfg.at("Id");

            if(traceEnabled) {
                LogPrintf(spdlog::level::info, "|TRK| Trace for pump |{}|: enabled", colId);
            } else {
                LogPrintf(spdlog::level::info, "|TRK| Trace for pump |{}|: disabled", colId);
            }
            traceStates[colId] = traceEnabled;
        }
        states.setDataValidityTime(cfgObj.at("data_validity_time"));
        // Battery level: in 6 minutes
        //    left: 62 -> 73
        //    right: 11 -> 32
        //    case: 3 -> 6

        lineCfg = ColumnConfig(cfgObj);
        connect = createConnection(conn_cfg, conn_type);
        createProtocol(protocol_type);
    } catch(const nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "Line contructor error: {}", e.what());
    } catch(const std::invalid_argument& e){
        LogPrintf(spdlog::level::err, "Line parse error: {}", e.what());
    }
}

void close() {
}

void ColumnLine::poll() {
    while(working) {
        for (const auto&[columnId, columnAddr]: lineCfg.columnIdAddr) {
            if (connect->getConnectionInited()) {
                if (columnCmds.size()) {
                    processCmd();
                } else {
                    processStatus(columnId, columnAddr);
                }
            } else {
                connect->closeConnection();
                connect->connectInit();
            }
        }
    }
    LogPrintf(spdlog::level::info, "Closing column poll");
}

bool ColumnLine::checkPacket(const std::vector<unsigned char>& cmd) {
    if(cmd.empty()) {
        return false;
    }
    if(cmd.size() < proto->getMinPacketSize()) {
        return false;
    }
    int responseColumnId = proto->getAddrOfPacket(cmd);

    /// Checking if column ID exists
    if(lineCfg.columnAddrId.find(responseColumnId) == lineCfg.columnAddrId.end()) {
        LogFile(spdlog::level::warn, "|TCP| Incorrect addr. Problematic packet: {:X}", fmt::join(cmd, " "));
        return false;
    }
    /// Checking CRC
    bool crcCorrect = proto->checkCheckSum(cmd);
    if(!crcCorrect) {
        LogFile(spdlog::level::warn, "CRC incorrect");
        return false;
    } else {
        return true;
    }
}

std::chrono::steady_clock::time_point getTimeNowColumnLine() {
    return std::chrono::steady_clock::now();
}

std::vector<unsigned char> concatTwoVectorsColumn(std::vector<unsigned char> A, std::vector<unsigned char> B) {
    std::vector<unsigned char> AB;

    AB.reserve( A.size() + B.size() ); // preallocate memory
    AB.insert( AB.end(), A.begin(), A.end() );
    AB.insert( AB.end(), B.begin(), B.end() );
    return AB;
}

std::unique_ptr<UniversalCmd> makeCmd(std::shared_ptr<UniversalState>& data) {
    int columnId = data->getId();

    switch(data->getType()) {
        case StateType::AUTHORIZE_REGISTERED: {
            auto authorizeResponse = std::static_pointer_cast<UniversalAuthorizeConfirmation>(data);
            const std::string user = fmt::format("pump%d", columnId);
            int nozzle = authorizeResponse->getNozzle();
            int volume = authorizeResponse->getVolume();
            return std::make_unique<UniversalAuthorize>(columnId, CmdType::WRITE_VOLUME, volume, nozzle, 0, user);
        }
        default:
            LogPrintf(spdlog::level::warn, "Incorrect state for converting to cmd: {}", data->getType());
            return std::make_unique<UniversalCmd>(columnId, CmdType::STATUS_REQUEST);
    }
}

void ColumnLine::processStatus(const ColumnId columnId, Addr columnAddr) {
    auto requestStatus = std::make_unique<UniversalCmd>(columnId, CmdType::STATUS_REQUEST);
    auto packet = proto->preparePacket(std::move(requestStatus), columnAddr);
    proto->traceRequests(packet);
    auto responsePacket = connect->transmit(packet, traceStates.at(columnId));

    if (checkPacket(responsePacket)) {
        if(connect->getUniqueTraces()) {
            proto->traceResponse(responsePacket);
        }
        auto addr = proto->getAddrOfPacket(responsePacket);
        int responseColumnId = lineCfg.columnAddrId.at(addr);
        auto responseState = proto->parseResponse(responsePacket, columnAddr, responseColumnId);
        if(responseState->getType() != StateType::ERROR and
        responseState->getType() != StateType::AUTHORIZE_REGISTERED and
        responseState->getType() != StateType::REMOVED_MANUAL_DOSING) {
            states.addState(columnId, responseState);
        } else if(responseState->getType() == StateType::AUTHORIZE_REGISTERED) {
            addCmd(makeCmd(responseState));
        }
    } else {
        bool traceActive = connect->getTraceLogsState() and traceStates.at(columnId) and !(connect->getUniqueTraces());
        auto tryEndPacket = connect->readPacket(traceActive);

        if (checkPacket(concatTwoVectorsColumn(responsePacket, tryEndPacket))) {
            responsePacket = concatTwoVectorsColumn(responsePacket, tryEndPacket);
            if(connect->getUniqueTraces()) {
                proto->traceResponse(responsePacket);
            }
            auto addr = proto->getAddrOfPacket(responsePacket);
            int responseColumnId = lineCfg.columnAddrId.at(addr);
            auto responseState = proto->parseResponse(responsePacket, columnAddr, responseColumnId);
            if(responseState->getType() != StateType::ERROR) {
                states.addState(columnId, responseState);
            }
        }
    }
}

void ColumnLine::processCmd() {
    auto &cmd = columnCmds.front();

    try {
        int id = cmd->getId();
        int addr = lineCfg.columnIdAddr.at(id);
        const std::string cmdUser = cmd->getControlUser();

        if(cmd->getCmdType() == CmdType::WRITE_VOLUME) {
            states.setUserUpdateValidity(id, false);
            auto authorizeCmd = std::make_unique<UniversalAuthorize>(*dynamic_cast<UniversalAuthorize*>(cmd.release()));
            auto setPriceCmd = std::make_unique<UniversalAuthorize>(authorizeCmd->getId(), CmdType::WRITE_MONEY, 0, 0, authorizeCmd->getPrice(), "");

            auto packet = proto->preparePacket(std::move(setPriceCmd), addr);
            proto->traceRequests(packet);
            auto responsePacket = connect->transmit(packet, traceStates.at(id));
            cmd = std::move(authorizeCmd);
        } else if(cmd->getCmdType() == CmdType::CLOSE_REPORT) {
            states.setUserUpdateValidity(id, false);
        }

        auto packet = proto->preparePacket(std::move(cmd), addr);
        proto->traceRequests(packet);
        auto responsePacket = connect->transmit(packet, traceStates.at(id));
        if (checkPacket(responsePacket)) {
            if(connect->getUniqueTraces()) {
                proto->traceResponse(responsePacket);
            }
            auto responseState = proto->parseResponse(responsePacket, addr, id);
            if(responseState->getType() != StateType::ERROR) {
                responseState->setControlUser(cmdUser);
                states.addState(id, std::dynamic_pointer_cast<UniversalState>(responseState));
            }
        } else {
            bool traceActive = connect->getTraceLogsState() and traceStates.at(id) and !(connect->getUniqueTraces());
            auto tryEndPacket = connect->readPacket(traceActive);

            if (checkPacket(concatTwoVectorsColumn(responsePacket, tryEndPacket))) {
                responsePacket = concatTwoVectorsColumn(responsePacket, tryEndPacket);
                if(connect->getUniqueTraces()) {
                    proto->traceResponse(responsePacket);
                }
                auto addrExtended = proto->getAddrOfPacket(responsePacket);
                int responseColumnId = lineCfg.columnAddrId.at(addrExtended);
                auto responseState = proto->parseResponse(responsePacket, id, responseColumnId);
                if(responseState->getType() != StateType::ERROR) {
                    responseState->setControlUser(cmdUser);
                    states.addState(id, responseState);
                }
            }
        }
    } catch (std::exception &e) {
        LogPrintf(spdlog::level::err, "ColumnLine::poll err: {}", e.what());
    }
    columnCmds.pop_front();
}

std::shared_ptr<UniversalState> ColumnLine::getState(ColumnId id) {
    return states.getState(id);
}

std::future<void> ColumnLine::start() {
    working = true;
    LogPrintf(spdlog::level::info, "Starting Column poller thread");
    connect->connectInit();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::future<void> pollFuture = std::async(std::launch::async, &ColumnLine::poll, shared_from_this());
    return pollFuture;
}

void ColumnLine::addCmd(std::unique_ptr<UniversalCmd> cmd) {
    columnCmds.push_back(std::move(cmd));
}

bool ColumnLine::getWorkingState() const {
    return working;
}

[[maybe_unused]] void ColumnLine::setWorkingState(bool state) {
    working = state;
}

[[maybe_unused]] void ColumnLine::addTechCmd(std::unique_ptr<UniversalCmd> cmd) {
    technicalCmds.push_back(std::move(cmd));
}
