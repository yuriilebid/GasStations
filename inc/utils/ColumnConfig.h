#ifndef PTS2_0_COLUMNCONFIG_H
#define PTS2_0_COLUMNCONFIG_H

#include <nlohmann/json.hpp>

#include "utils/logger.h"
#include "FuelProduct.h"

using ColumnId = int;
using ProductCode = int;
using line = int;
using Addr = int;

class ColumnConfig {
public:
    std::map<ColumnId, Addr> columnIdAddr;
    std::map<Addr, ColumnId> columnAddrId;
    std::map<ColumnId, std::vector<ProductCode>> columnIdProductCode;
    std::map<ProductCode, FuelProduct> productIdProduct; /// TODO <- тут второе использование productConfig

    ColumnConfig() = default;

    explicit ColumnConfig(const nlohmann::json& jsonObj) {
        for(auto& column : jsonObj.at("columns")) {
            try {
                int columnId = column.at("Id");
                int columnAddr = column.at("addr");
                std::vector<int> productCode;

                for(auto& pistol : column.at("pistols")) {
                     productCode.push_back(pistol.at("product_id"));
                }

                columnIdAddr.emplace(columnId, columnAddr);
                columnAddrId.emplace(columnAddr, columnId);
                LogPrintf(spdlog::level::info, "Adding to column Product on ID: {}", productCode.size(), columnId);
                columnIdProductCode.emplace(columnId, productCode);
            } catch(nlohmann::json::exception& e){
                LogPrintf(spdlog::level::warn, "Error parsing line cfg: {}", e.what());
            }
        }
    }
};

#endif //PTS2_0_COLUMNCONFIG_H
