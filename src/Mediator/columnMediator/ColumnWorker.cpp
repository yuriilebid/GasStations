#include "ColumnWorker.h"
#include "ColumnLine.h"
#include "UniversalGradesConfig.h"
#include "UniversalPumpsConfig.h"

std::shared_ptr<UniversalState> ColumnWorker::getState(int id) {
    if(lineList.find(id) != lineList.end()) {
        return lineList.at(id)->getState(id);
    }
    return std::make_shared<UniversalState>(id, 0, StateType::NOT_SERVED, 0);
}

void ColumnWorker::changeLog(columnId _id, bool logTraceStatus) {
    lineList.at(_id)->changeLog(_id, logTraceStatus);
}

std::unique_ptr<UniversalTech> ColumnWorker::operateTechCmd(std::unique_ptr<TechCmd> cmd) {
    switch(cmd->getCmdType()) {
        case CmdType::GRADES_CONFIG:
            LogPrintf(spdlog::level::info, "|GRADES_CONFIG| technical cmd");
            return std::make_unique<UniversalGradesConfig>(fuelCfg);
        case CmdType::PUMP_CONFIG: {
            LogPrintf(spdlog::level::info, "|PUMP_CONFIG| technical cmd");
            std::map<int, std::vector<int>> pumpsData;

            for (auto&[columnId, line]: lineList) {
                auto pumpsProducts = line->lineCfg.columnIdProductCode.at(columnId);

                pumpsData[columnId] = pumpsProducts;
            }
            std::unique_ptr<UniversalPumpsConfig> cfg = std::make_unique<UniversalPumpsConfig>(pumpsData);
            return cfg;
        }
        case CmdType::WRITE_PRICE: {
            auto cmdPrice = std::make_unique<UniversalPrice>(*dynamic_cast<UniversalPrice*>(cmd.release()));
            auto pricesConfiguration = cmdPrice->getPrices();

            LogPrintf(spdlog::level::info, "|WRITE_PRICE| technical cmd");
            for (auto&[columnId, line]: lineList) {
                for(auto&[fuelId, updatedProduct] : pricesConfiguration) {
                    line->lineCfg.productIdProduct.insert({fuelId, updatedProduct});
                }
            }
            return std::make_unique<UniversalTech>(techCmdType::SET_PRICE);
        }
        default:
            LogPrintf(spdlog::level::err, "NO technical cmd");
            break;
    }
    return std::make_unique<UniversalTech>(techCmdType::BATTERY);
}

void ColumnWorker::addCmd(std::unique_ptr<UniversalCmd> cmd) {
    int columnId = cmd->getId();

    lineList.at(columnId)->addCmd(std::move(cmd));
}

std::vector<std::future<void>> ColumnWorker::start() {
    std::vector<std::future<void>> linesFutures;
    LogPrintf(spdlog::level::info, "Starting Column Worker");

    for (const auto&[columnId, line]: lineList) {
        LogPrintf(spdlog::level::info, "Checking working state");
        if(!line->getWorkingState()) {
            LogPrintf(spdlog::level::info, "pushing line start");
            linesFutures.push_back(line->start());
        }
    }
    return linesFutures;
}
