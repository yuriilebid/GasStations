#ifndef PTS2_0_UNIVERSALAMOUNT_H
#define PTS2_0_UNIVERSALAMOUNT_H

#include "UniversalState.h"

/**
 *  ̶ ̶
    ̶ Adast
    ̶ ̶ ̶
    2 bytes – binary number of the transaction; the higher order bits are sent in the first byte,
    1 decimal ASCII digit character – nozzle number; digit from the range 1 to 6,
    1 hexadecimal ASCII digit character – product code,
    1 ASCII character – N/P; transaction type
    • P – prepay specified by the service station console,
    • N – normal (including the user keyboard specified transaction),
    7 ASCII characters – current total price of the transaction including decimal point,
    7 ASCII characters – current requestedVolume of the transaction including decimal point,
    5 ASCII characters – unit price of the transaction including decimal point.
 */

/** Unipump
    10 => <DLE>
    02 => <STX>
    31 => <ADDRESS> ‘1’
    41 => <RESPONSE CODE>
    30 32 => number of transaction (2)
    31 => number of active nozzle (1)
    30 30 30 30 30 32 35 30 => dispensed in requestedVolume units (2500
    ml)
    77 41 => <CRC>
    10 => <DLE>
    03 => <ETX>
 */



class UniversalAmount : public UniversalState {
private:
    int volume{};
    int price{};
    int sum{};
    int dbTransactionId{};
    int productCode{};
public:
    UniversalAmount(int _address, int _nozzle, int _productCode, int _volume, int _price,
                    int _transactionId, StateType type, int _sum);
    UniversalAmount(): UniversalState(0, 0, StateType::FUEL_SUPPLY, 0) {};

    UniversalAmount& setVolume(int);
    UniversalAmount& setPrice(int);
    UniversalAmount& setSum(int);
    UniversalAmount& setTxnId(int);
    UniversalAmount& setProductCode(int);
    int getVolume();
    int getPrice();
    int getSum();
    int getDbTransactionId();
    int getProductCode();
};


#endif //PTS2_0_UNIVERSALAMOUNT_H
