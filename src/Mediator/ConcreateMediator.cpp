#include <LogCmd.h>
#include <wamp.h>
#include "ConcreateMediator.h"
#include "UniversalTech.h"
#include "JsonWorker.h"
#include "TechCmd.h"

void ConcreteMediator::addCmd(std::unique_ptr<UniversalCmd> cmd) const {
    columnWorker->addCmd(std::move(dataBase->updateTransactionFromCmd(std::move(cmd))));
}

std::unique_ptr<UniversalTech> ConcreteMediator::operateCmd(std::unique_ptr<TechCmd> cmd) const {
    return columnWorker->operateTechCmd(std::move(cmd));
}

std::map<int, NozzleLogicState> ConcreteMediator::checkUnfinishedKassaSimulations(const std::vector<int>& listOfPumps) const {
    LogPrintf(spdlog::level::info, "|PTS| Checking for undone simulations");
    return this->dataBase->checkUnfinishedSimulations(listOfPumps);
}

void ConcreteMediator::checkUndoneTransactions(const nlohmann::json &cfg) const {
    LogPrintf(spdlog::level::info, "Checking for undone transactions");
    this->kassaWorker->checkUndoneTransactions(cfg);
}

std::unique_ptr<UniversalTech> ConcreteMediator::setPrice(std::unique_ptr<UniversalPrice> cmd) const {
    return columnWorker->operateTechCmd(std::move(cmd));
}

void ConcreteMediator::end() {
    for(auto& asyncTd : futuresList) {
        asyncTd.get();
    }
}

void ConcreteMediator::start() {
    futuresList.push_back(std::move(std::async(std::launch::async, &JsonWorker::start, jsonWorker)));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto columnsFutures = columnWorker->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto kassaFutures = kassaWorker->start();

    for(auto& colFuture : columnsFutures) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        futuresList.push_back(std::move(colFuture));
    }
    for(auto& kassaFuture : kassaFutures) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        futuresList.push_back(std::move(kassaFuture));
    }
}

void ConcreteMediator::changeScenario(std::unique_ptr<UniversalCmd> cmd) const {
    kassaWorker->changeLogicType(std::move(std::make_unique<TechCmd>(*dynamic_cast<TechCmd*>(cmd.release()))));
}

std::string ConcreteMediator::getLicenseKey() const {
    return dataBase->getLicenseKey();
}

void ConcreteMediator::updateLicenseKet(const std::string updatedLicenseKey) const {
    return dataBase->updateLicenseKey(updatedLicenseKey);
}

void ConcreteMediator::changeLog(std::map<columnId, bool> &&logLevelMap) const {
    for(const auto& elem : logLevelMap) {
        columnWorker->changeLog(elem.first, elem.first);
    }
}

std::shared_ptr<UniversalTotals> ConcreteMediator::getKassaTotals(std::unique_ptr<UniversalCmd> cmd) const {
    return this->dataBase->getKassaTotals(std::move(cmd));
}

void ConcreteMediator::addKassaTotals(std::shared_ptr<UniversalState> data) const {
    this->dataBase->addTotalKassaInfo(std::static_pointer_cast<UniversalTotals>(data));
}

std::unique_ptr<UniversalCmd>
ConcreteMediator::checkKassaUpdates(std::unique_ptr<UniversalCmd> data, const std::string &user) const {
    switch(data->getCmdType()) {
        case CmdType::CHANGE_SCENARIOS:
            return this->dataBase->addSimulation(std::move(data), user);
        case CmdType::WRITE_VOLUME:
        case CmdType::STOP_COMMAND:
            return this->dataBase->updateSimulationFromCmd(std::move(data), user);
        default:
            return data;
    }
}

void ConcreteMediator::checkDbUpdates(std::shared_ptr<UniversalState> data) const {
    switch(data->getType()) {
        case StateType::SUPPLY_DONE:
        case StateType::FUEL_SUPPLY:
            this->dataBase->updateTransaction(data);
            break;
        default:
            break;
    }
}

std::shared_ptr<UniversalState> ConcreteMediator::getState(int columnId) const {
    auto resState = columnWorker->getState(columnId);
    auto responseState = resState->getType();

    checkDbUpdates(resState);
    return resState;
}

ConcreteMediator::ConcreteMediator(const nlohmann::json& cfgObj) {
    dataBase = std::make_unique<TransactionControl>();
    columnWorker = std::make_unique<ColumnWorker>(cfgObj);
    jsonWorker = JsonWorker::create(cfgObj);
    jsonWorker->parseConfig(cfgObj);
    kassaWorker = KassaWorker::create(cfgObj);
    kassaWorker->parseCfg(cfgObj);
//    wampWorker = std::make_unique<Wamp>();
}

std::shared_ptr<ConcreteMediator> ConcreteMediator::create(const nlohmann::json& cfgObj) {
    return std::shared_ptr<ConcreteMediator>(new ConcreteMediator(cfgObj));
}

void ConcreteMediator::setAllMediator() {
    LogPrintf(spdlog::level::info, "Setting mediator to columnWorker");
    this->columnWorker->setMediator(shared_from_this());
    LogPrintf(spdlog::level::info, "Setting mediator to jsonWorker");
    this->jsonWorker->setMediator(shared_from_this());
    LogPrintf(spdlog::level::info, "Setting mediator to kassaWorker");
    this->kassaWorker->setMediator(shared_from_this());
    LogPrintf(spdlog::level::info, "Setting mediator to DB");
    this->dataBase->setMediator(shared_from_this());
}

ConcreteMediator::~ConcreteMediator() = default;

std::unique_ptr<UniversalCmd>
ConcreteMediator::jsonCheck(std::unique_ptr<UniversalCmd> cmd, const std::string &user) const {
    switch(cmd->getCmdType()) {
        case CmdType::WRITE_VOLUME: {
            return dataBase->addTransaction(std::move(cmd), user);
        }
        default:
            return cmd;
    }
}

int ConcreteMediator::gettransactionDbId(std::shared_ptr<UniversalState>& responseStae) const {
    return dataBase->getTransactionId(responseStae);
}

void ConcreteMediator::fiscalCheck(std::unique_ptr<UniversalCmd> cmd) const {
    switch(cmd->getCmdType()) {
        case CmdType::WRITE_VOLUME: {
            const std::string user = "FISCAL";

            dataBase->addTransaction(std::move(cmd), user);
        }
            break;
        default:
            break;
    }
}

FuelProduct ConcreteMediator::getProductOnPumpNozzle(int pump, int nozzle) const {
    return columnWorker->getProductByPumpNozzle(pump, nozzle);
}
