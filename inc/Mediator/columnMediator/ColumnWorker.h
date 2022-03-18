#ifndef PTS2_0_COLUMNWORKER_H
#define PTS2_0_COLUMNWORKER_H

#include "Mediator.h"
#include <map>
#include "TechCmd.h"
#include "ColumnLine.h"
#include "UniversalTech.h"
#include "FuelProduct.h"
#include <map>

using ColumnId = int;
using addr = int;
using productCode = int;

using namespace nlohmann;

class ColumnWorker : public BaseComponent {
private:
    std::map<productCode, FuelProduct> fuelCfg;
    std::map<ColumnId, std::shared_ptr<ColumnLine>> lineList;
public:
    explicit ColumnWorker(const nlohmann::json& cfgJson) {
        LogPrintf(spdlog::level::info, "Parsing config in ColumnWorker");
        parseCfg(cfgJson);
    }

    FuelProduct getProductByPumpNozzle(int pump, int nozzle) {
        auto productId = lineList.at(pump)->lineCfg.columnIdProductCode.at(pump).at(nozzle - 1);
        return fuelCfg.at(productId);
    }

    void parseCfg(const nlohmann::json& cfgJson) {
        LogPrintf(spdlog::level::info, "Parsing ColumnWorker");

        try {
            for(auto& product : cfgJson.at("fuelGradesConfiguration")) {
                int productId = product.at("Id");
                int productPrice = 1000;
                if(product.contains("Price")) {
                    productPrice = product.at("Price");
                }
                const std::string productName = product.at("Name");
                fuelCfg.insert({productId, {productId, productPrice, productName}});
            }
            for(auto& line : cfgJson.at("lines")) {
                LogPrintf(spdlog::level::info, "Creating new line");
                const auto linePtr = ColumnLine::create(line);

                LogPrintf(spdlog::level::info, "Line created");
                linePtr->setConfig(fuelCfg);
                LogPrintf(spdlog::level::info, "Config setted");
                for(auto& column : line.at("columns")) {
                    const int lineId = column.at("Id");

                    LogPrintf(spdlog::level::info, "Adding column with ID |{}|", lineId);
                    lineList[lineId] = linePtr;
                }
            }
        } catch(nlohmann::json::exception& e) {
            LogPrintf(spdlog::level::err, "JsonLine constructor error ", e.what());
        }
    }
    std::shared_ptr<UniversalState> getState(int id);
    void changeLog(columnId _id, bool logTraceStatus);
    std::unique_ptr<UniversalTech> operateTechCmd(std::unique_ptr<TechCmd> cmd);
    void addCmd(std::unique_ptr<UniversalCmd> cmd);
    std::vector<std::future<void>> start();
};


#endif //PTS2_0_COLUMNWORKER_H
