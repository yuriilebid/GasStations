#ifndef PTS2_0_UNIVERSALSETPRICE_H
#define PTS2_0_UNIVERSALSETPRICE_H

#include "UniversalCmd.h"

class UniversalSetPrice : public UniversalCmd {
private:
    int price{};
public:
    UniversalSetPrice(int _columnId, CmdType _type, int _price);
};


#endif //PTS2_0_UNIVERSALSETPRICE_H
