#include "UniversalPumpsConfig.h"

#include <utility>
#include "logger.h"

void UniversalPumpsConfig::addPump(int pump, const std::vector<int>& products) {
    LogPrintf(spdlog::level::info, "Adding VECTOR size: {} on pump: {}", products.size(), pump);
    std::map<pumpId, std::vector<productId>> tmpSample;

    pumpConfig[pump] = products;
    LogPrintf(spdlog::level::info, "Added second");
}

std::map<pumpId, std::vector<productId>> UniversalPumpsConfig::getPumpsConfig() {
    return pumpConfig;
}