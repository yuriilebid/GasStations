#include "KassaWorker.h"
#include "TechCmd.h"
#include "TcpConnection.h"
#include "serialConnection.h"
#include "ProtocolSlaveUnipump.h"
#include "ProtocolSlaveAdast.h"
#include "ProtocolSlaveShelf.h"

KassaWorker::KassaWorker(const nlohmann::json &cfgObj) {
    workingCondition = false;
    logicType = LogicType::IDLE;
    LogPrintf(spdlog::level::info, "Parsing config in KassaWorker");
}

std::vector<std::future<void>> KassaWorker::start() {
    LogPrintf(spdlog::level::info, "Starting Kassa Worker");
    std::vector<std::future<void>> kassaTds;

    for(auto& fiscaler : kassaLines) {
        kassaTds.push_back(fiscaler->start());
    }
    return kassaTds;
}

std::shared_ptr<FiscalLine> FiscalLine::create(const nlohmann::json& cfgObj, const std::shared_ptr<KassaWorker>& fiscaler) {
    return std::shared_ptr<FiscalLine>(new FiscalLine(cfgObj, fiscaler));
}

void KassaWorker::checkUndoneTransactions(const nlohmann::json &cfg) {
    std::vector<int> allColumnsId;

    try {
        for (auto &line: cfg.at("lines")) {
            for (auto &col: line.at("columns")) {
                allColumnsId.push_back(col.at("Id"));
            }
        }
        auto unDoneStates = this->mediator_->checkUnfinishedKassaSimulations(allColumnsId);
        /// TODO: temporary undefined
//        for (auto&[pump, state]: unDoneStates) {
//            scenarios->addState(pump, state);
//        }
    } catch (std::out_of_range &e) {
        LogPrintf(spdlog::level::warn, "|checkUndoneTransactions| error: {}", e.what());
    }
}

void KassaWorker::parseCfg(const nlohmann::json &cfgObj) {
    try {
        for(auto& fiscalKassa : cfgObj.at("fiscals")) {
            auto fiscalUser = fiscalKassa.at("user");
            scenarios[fiscalUser] = std::make_unique<Scenario>(cfgObj);
            LogPrintf(spdlog::level::info, "Adding fiscal device");
            auto tmpFiscalLine = FiscalLine::create(fiscalKassa, shared_from_this());
            kassaLines.push_back(tmpFiscalLine);
        }
    } catch (const std::exception& e) {
        LogPrintf(spdlog::level::err, "Parsing config for Kassa failed: {}", e.what());
    }
}

std::shared_ptr<UniversalState>
KassaWorker::processCmd(std::unique_ptr<UniversalCmd> cmd, const std::string &fiscalUser) {
//    if(cmd->getCmdType() == CmdType::GET_TOTAL) {
//        LogPrintf(spdlog::level::info, "|PTS| Processing total info from |FISCAL|");
//        return this->mediator_->getKassaTotals(std::move(cmd));
//    }
    auto response = scenarios.at(fiscalUser)->processCmd(std::move(cmd));

    if(response->getType() == StateType::SUPPLY_DONE) {
        this->mediator_->addKassaTotals(response);
    }
    return response;
}

std::shared_ptr<KassaWorker> KassaWorker::create(const nlohmann::json& cfgObj) {
    return std::shared_ptr<KassaWorker>(new KassaWorker(cfgObj));
}

void KassaWorker::changeLogicType(std::unique_ptr<TechCmd> cmd) {
    for(auto& eachScenario : scenarios) {
        cmd = eachScenario.second->changeLogicType(std::move(cmd));
    }
}

void FiscalLine::parseConfig(const nlohmann::json& cfgobj) {
    try {
        auto& conn_cfg = cfgobj.at("communication");
        auto conn_type = conn_cfg.at("transportation").get<ConnectionType>();
        auto protoType = cfgobj.at("protocol").get<ColumnProtocolType>();
        user = cfgobj.at("user");
        if(cfgobj.at("fiscal") == "maria") {
            LogPrintf(spdlog::level::info, "|FISCAL| Maria");
            type = FiscalType::MARIA;
        } else {
            LogPrintf(spdlog::level::info, "|FISCAL| Techno");
            type = FiscalType::TECHNO;
        }

        switch(conn_type) {
            case ConnectionType::TCP:
                connect = std::make_unique<TcpConnection>(conn_cfg);
                break;
            case ConnectionType::SERIAL:
                connect = std::make_unique<SerialConnection>(conn_cfg);
                break;
            default:
                LogPrintf(spdlog::level::err, "Undefined transportation for Fiscal dev");
                break;
        }
        switch(protoType) {
            case ColumnProtocolType::UNIPUMP:
                proto = std::make_unique<ProtocolSlaveUnipump>();
                break;
            case ColumnProtocolType::SHELF:
                proto = std::make_unique<ProtocolSlaveShelf>();
                break;
            case ColumnProtocolType::ADAST:
                proto = std::make_unique<ProtocolSlaveAdast>();
                break;
            default:
                LogPrintf(spdlog::level::err, "Undefined protocol for Fiscal dev");
                break;
        }
        proto->type = type;

    } catch(const nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "Fiscal device configuration: {}", e.what());
        exit(1);
    }
}

FiscalLine::FiscalLine(const nlohmann::json& cfgObj, std::shared_ptr<KassaWorker> fiscaler): Line(), fiscalWorker(std::move(fiscaler)) {
    parseConfig(cfgObj);
}

std::future<void> FiscalLine::start() {
    pollingCondition = true;
    LogPrintf(spdlog::level::info, "Starting FiscalLine thread");
    std::future<void> pollFuture = std::async(std::launch::async, &FiscalLine::poll, shared_from_this());
    return pollFuture;
}

std::vector<unsigned char> concatTwoVectors(std::vector<unsigned char> A, std::vector<unsigned char> B) {
    std::vector<unsigned char> AB;

    AB.reserve( A.size() + B.size() ); // preallocate memory
    AB.insert( AB.end(), A.begin(), A.end() );
    AB.insert( AB.end(), B.begin(), B.end() );
    return AB;
}

void FiscalLine::poll() {
    std::vector<unsigned char> oldBadPacket {};

    while(pollingCondition) {
        if(connect->getConnectionInited()) {
            auto receivedPacket = connect->readPacket(true);
//            LogPrintf(spdlog::level::info, "|PACKET KASSA| {:X}", fmt::join(receivedPacket, " "));
            if(proto->checkPacketAppropriation(receivedPacket)) {
                proto->traceResponse(receivedPacket);
                auto receivedCmd = proto->parseCmd(receivedPacket);
                auto cmd = fiscalWorker->mediator_->checkKassaUpdates(std::move(receivedCmd), user);
                auto state = fiscalWorker->processCmd(std::move(cmd), user);
                auto feedbackPacket = proto->preparePacketVec(state, 0);

                proto->traceRequests(feedbackPacket);
                connect->writePacket(feedbackPacket, true);
            } else if(proto->checkPacketAppropriation(concatTwoVectors(oldBadPacket, receivedPacket))) {
                receivedPacket = concatTwoVectors(oldBadPacket, receivedPacket);
                proto->traceResponse(receivedPacket);
//                LogPrintf(spdlog::level::info, "EXTENDED Packet: {}", fmt::join(receivedPacket, " "));
                auto receivedCmd = proto->parseCmd(receivedPacket);
                auto cmd = fiscalWorker->mediator_->checkKassaUpdates(std::move(receivedCmd), user);
                auto state = fiscalWorker->processCmd(std::move(cmd), user);
                auto feedbackPacket = proto->preparePacketVec(state, 0);

                proto->traceRequests(feedbackPacket);
                connect->writePacket(feedbackPacket, true);
            } else {
                oldBadPacket.erase(oldBadPacket.begin(), oldBadPacket.begin());
                oldBadPacket = receivedPacket;
            }
        } else {
//            connect->closeConnection();
            connect->connectInit();
        }
    }
}
