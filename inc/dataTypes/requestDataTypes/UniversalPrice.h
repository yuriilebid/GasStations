#ifndef PTS2_0_UNIVERSALPRICE_H
#define PTS2_0_UNIVERSALPRICE_H

#include "TechCmd.h"
#include "FuelProduct.h"
#include <map>

using ProductCode = int;
using PriceInCents = int;

class UniversalPrice : public TechCmd {
private:
    std::map<ProductCode, FuelProduct> prices;
public:
    void addProduct(int productCode, const std::string& name, int price) {
        prices.insert({productCode, {productCode, price, name}});
    }

    std::map<int, FuelProduct> getPrices() {
        return prices;
    }
};

#endif //PTS2_0_UNIVERSALPRICE_H
