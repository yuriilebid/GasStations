#include "FuelProduct.h"

int FuelProduct::getPrice() const {
    return price;
}

int FuelProduct::getId() const {
    return id;
}

std::string FuelProduct::getName() const {
    return name;
}

void FuelProduct::setId(int _id) {
    id = _id;
}

void FuelProduct::setPrice(int _price) {
    price = _price;
}

void FuelProduct::setName(const std::string& _name) {
    name = _name;
}
