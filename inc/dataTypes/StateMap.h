#ifndef PTS2_0_STATEMAP_H
#define PTS2_0_STATEMAP_H

#include <mutex>
#include <map>
#include <memory>
#include <atomic>
#include <chrono>

#include "UniversalState.h"

using ColumnId = int;

class StateMap {
    std::map<int, bool> userUpdateAvailable;
    int dataValidityTime {};
    std::mutex stateMx;
    std::map<ColumnId, std::chrono::steady_clock::time_point> timeStamps;
    std::map<ColumnId, std::shared_ptr<UniversalState>> states;
public:
    static std::chrono::steady_clock::time_point getTimeNow();
    std::chrono::steady_clock::time_point getTime(int colId);
    void setDataValidityTime(int timeSeconds);
    void setTime(int colId);
    void setUserUpdateValidity(bool stateUpdate, int _pumpId);
    void LogInfo(ColumnId id, const std::shared_ptr<UniversalState>& newState);
    bool checkStateActuality(int colId);
    bool getUserUpdateValidity(int _pumpId);
    void addState(ColumnId id, const std::shared_ptr<UniversalState>& newState);
    std::shared_ptr<UniversalState> getState(ColumnId id);
};


#endif //PTS2_0_STATEMAP_H
