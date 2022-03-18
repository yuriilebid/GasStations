#ifndef PTS2_0_UNIVERSALAUTHORIZECONFIRMATION_H
#define PTS2_0_UNIVERSALAUTHORIZECONFIRMATION_H

#include "UniversalState.h"

class UniversalAuthorizeConfirmation : public UniversalState {
private:
    int transactionDbId{};
    int volume{};
    int price{};
public:
    UniversalAuthorizeConfirmation(int id): transactionDbId(id) {
        setType(StateType::AUTHORIZE_REGISTERED);
    }
    UniversalAuthorizeConfirmation(int addr, int _volume, StateType _type) {
        setType(_type);
        setId(addr);
        volume = _volume;
    }

    int getVolume() {
        return volume;
    }

    void setVolume(int _volume) {
        volume = _volume;
    }

    int getPrice() {
        return price;
    }

    void setPrice(int _price) {
        price = _price;
    }

    int getTransasctionDbId() {
        return transactionDbId;
    }

    void setTransactionDbId(int id) {
        transactionDbId = id;
    }
};


#endif //PTS2_0_UNIVERSALAUTHORIZECONFIRMATION_H
