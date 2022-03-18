#include "UniversalAuthorize.h"

#include <utility>

UniversalAuthorize::UniversalAuthorize(int _columnId, CmdType _type, int _volume, int _nozzle, int _price,
                                       std::string _user) :
UniversalCmd(_columnId, _type),
nozzle(_nozzle), volume(_volume), price(_price), user(std::move(_user)) {}

int UniversalAuthorize::getVolume() const {
    return volume;
}

void UniversalAuthorize::setVolume(int _volume) {
    volume = _volume;
}

void UniversalAuthorize::setUser(const std::string& _user) {
    user = _user;
}

int UniversalAuthorize::getNozzle() const {
    return nozzle;
}

void UniversalAuthorize::setNozzle(int _nozzle) {
    nozzle = _nozzle;
}

int UniversalAuthorize::getPrice() const {
    return price;
}

std::string UniversalAuthorize::getUser() const {
    return user;
}

void UniversalAuthorize::setPrice(int _price) {
    price = _price;
}

void UniversalAuthorize::setDbTransactionId(int _id) {
    dBtransactionId = _id;
}

int UniversalAuthorize::getDbTransactionId() const {
    return dBtransactionId;
}