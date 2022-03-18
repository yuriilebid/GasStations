#ifndef PTS2_0_UNIVERSALCLOSE_H
#define PTS2_0_UNIVERSALCLOSE_H

#include "UniversalCmd.h"

class UniversalClose : public UniversalCmd {
private:
    int transactionId;
public:
    UniversalClose() {
        setCmdType(CmdType::CLOSE_REPORT);
    }
    int getTransId() const;
    void setTransactionId(int txnId);
};

#endif //PTS2_0_UNIVERSALCLOSE_H
