#ifndef PTS2_0_UNIVERSALTOTAL_H
#define PTS2_0_UNIVERSALTOTAL_H

#include "UniversalCmd.h"

class UniversalTotal : public UniversalCmd {
private:
    int nozzle {};
public:
    UniversalTotal(int _nozzle) : nozzle(_nozzle) {
        setCmdType(CmdType::GET_TOTAL);
    }
    int getNozzle() {
        return nozzle;
    }
};


#endif //PTS2_0_UNIVERSALTOTAL_H
