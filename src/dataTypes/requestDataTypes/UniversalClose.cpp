#include "UniversalClose.h"

int UniversalClose::getTransId() const {
    return transactionId;
}

void UniversalClose::setTransactionId(int txnId) {
    transactionId = txnId;
}