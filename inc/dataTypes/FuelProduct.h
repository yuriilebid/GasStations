#ifndef PTS2_0_FUELPRODUCT_H
#define PTS2_0_FUELPRODUCT_H

#include <string>
#include <utility>

class FuelProduct {
private:
    int id;
    int price;
    std::string name;
public:
    FuelProduct(int _id, int _price, std::string  _name) : id(_id), price(_price), name(std::move(_name)) {};

    int getPrice() const;
    int getId() const;
    std::string getName() const;
    void setId(int _id);
    void setPrice(int _price);
    void setName(const std::string& _name);
};


#endif //PTS2_0_FUELPRODUCT_H
