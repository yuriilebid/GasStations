#include "UniversalTotals.h"

void UniversalTotals::setVolume(long long newVolume) {
    volume = newVolume;
}

UniversalTotals::UniversalTotals(long long _volume, long long _sum) {
    volume = _volume;
    sum = _sum;
    setType(StateType::TOTAL_RESPONSE);
}

UniversalTotals::UniversalTotals(int addr, long long _volume, long long _sum, int nozzle) {
    volume = _volume;
    sum = _sum;
    setId(addr);
    setType(StateType::TOTAL_RESPONSE);
    setNozzle(nozzle);
}

void UniversalTotals::setSum(long long newSum) {
    sum = newSum;
}

long long UniversalTotals::getVolume() const {
    return volume;
}

long long UniversalTotals::getSum() const {
    return sum;
}
