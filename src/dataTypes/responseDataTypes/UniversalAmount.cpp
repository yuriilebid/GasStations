#include "UniversalAmount.h"

UniversalAmount::UniversalAmount(int _address, int _nozzle, int _productCode, int _volume, int _price,
                                 int _transactionId, StateType type, int _sum) :
        UniversalState(_address, _nozzle, type, 0),
        volume(_volume), price(_price), sum(_sum), dbTransactionId(_transactionId), productCode(_productCode){
    setType(type);
}

UniversalAmount& UniversalAmount::setVolume(int newVolume){
 volume = newVolume;
 return *this;
}
UniversalAmount& UniversalAmount::setPrice(int newPrice){
    price = newPrice;
    return *this;
}
UniversalAmount& UniversalAmount::setSum(int newSum){
    sum = newSum;
    return *this;
}
UniversalAmount& UniversalAmount::setTxnId(int newTxnId){
    dbTransactionId = newTxnId;
    return *this;
}
UniversalAmount& UniversalAmount::setProductCode(int newProductCode){
    productCode = newProductCode;
    return *this;
}

int UniversalAmount::getVolume() {
    return volume;
}

int UniversalAmount::getPrice() {
    return price;
}

int UniversalAmount::getSum() {
    return sum;
}

int UniversalAmount::getDbTransactionId() {
    return dbTransactionId;
}

int UniversalAmount::getProductCode() {
    return productCode;
}
