#include "UniversalProducts.h"

UniversalProducts::UniversalProducts(int _columnId, int _activeNozzle, StateType _type, int txnId, int txnType, int _volume, int _sum)
    : UniversalState(_columnId, _activeNozzle, _type, 0),
    transactionId(txnId), transactionType(txnType), volume(_volume), sum(_sum){}

UniversalProducts& UniversalProducts::setTxnId(int _txnId) {
    transactionId = _txnId;
    return *this;
}

UniversalProducts& UniversalProducts::setTxnType(int txnType) {
    transactionType = txnType;
    return *this;
}

UniversalProducts& UniversalProducts::setVolume(int _volume) {
    volume = _volume;
    return *this;
}

UniversalProducts& UniversalProducts::setSum(int _sum) {
    sum = _sum;
    return *this;
}
