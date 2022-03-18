#include "UniversalSetPrice.h"

UniversalSetPrice::UniversalSetPrice(int _columnId, CmdType _type, int _price):
        UniversalCmd(_columnId, _type), price(_price) {}