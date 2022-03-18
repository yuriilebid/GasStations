#ifndef PTS2_0_UNIVERSALPRODUCTS_H
#define PTS2_0_UNIVERSALPRODUCTS_H

#include <map>
#include "UniversalState.h"

class UniversalProducts : public UniversalState {
private:
    int transactionId{};
    int transactionType{};
    int volume{};
    int sum{};
public:
    UniversalProducts(int columnId, int activeNozzle, StateType _type, int txnId, int txnType, int _volume, int _sum);

    UniversalProducts& setTxnId(int _txnId);
    UniversalProducts& setTxnType(int txnType);
    UniversalProducts& setVolume(int _volume);
    UniversalProducts& setSum(int _sum);
};


#endif //PTS2_0_UNIVERSALPRODUCTS_H
