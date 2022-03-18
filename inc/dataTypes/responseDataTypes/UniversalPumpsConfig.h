#ifndef PTS2_0_UNIVERSALPUMPSCONFIG_H
#define PTS2_0_UNIVERSALPUMPSCONFIG_H

#include "UniversalTech.h"
#include <map>
#include <vector>
#include <memory>

using pumpId = int;
using productId = int;

class UniversalPumpsConfig : public UniversalTech {
private:
    std::map<pumpId, std::vector<productId>> pumpConfig {};
public:
    UniversalPumpsConfig() {
        setType(techCmdType::PUMP_CONFIGURATION);
    }
    explicit UniversalPumpsConfig(std::map<int, std::vector<int>> data) {
        setType(techCmdType::PUMP_CONFIGURATION);
        pumpConfig = data;
    }
    void addPump(int pump, const std::vector<int>& products);
    std::map<pumpId, std::vector<productId>> getPumpsConfig();
};


#endif //PTS2_0_UNIVERSALPUMPSCONFIG_H
