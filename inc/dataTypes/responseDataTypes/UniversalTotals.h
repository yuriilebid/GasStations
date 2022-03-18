#ifndef PTS2_0_UNIVERSALTOTALS_H
#define PTS2_0_UNIVERSALTOTALS_H

#include "UniversalState.h"

class UniversalTotals : public UniversalState {
private:
    long long volume{};
    long long sum{};
public:
    UniversalTotals() : UniversalState(0, 0, StateType::TOTAL_RESPONSE, 0) {}
    UniversalTotals(long long _volume, long long _sum);
    UniversalTotals(int addr, long long _volume, long long _sum, int nozzle);
    void setVolume(long long newVolume);
    void setSum(long long newSum);
    long long getVolume() const;
    long long getSum() const;
};


#endif //PTS2_0_UNIVERSALTOTALS_H
