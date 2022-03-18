#ifndef PTS2_0_UNIVERSALGRADESCONFIG_H
#define PTS2_0_UNIVERSALGRADESCONFIG_H

#include "UniversalState.h"
#include "UniversalTech.h"
#include "FuelProduct.h"
#include <map>
#include <utility>

using productCode = int;

class UniversalGradesConfig : public UniversalTech {
private:
    std::map<productCode, FuelProduct> productsConfiguration;
public:
    UniversalGradesConfig(std::map<int, FuelProduct> cfg) : productsConfiguration(std::move(cfg)) {
        setType(techCmdType::CONFIGURATION);
    }

    void setProductsConfiguration(std::map<int, FuelProduct> cfg) {
        productsConfiguration = cfg;
    }

    std::map<int, FuelProduct> getProductsConfiguration() {
        return productsConfiguration;
    }
};


#endif //PTS2_0_UNIVERSALGRADESCONFIG_H
