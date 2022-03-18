#ifndef PTS2_0_UNIVERSALAUTHORIZE_H
#define PTS2_0_UNIVERSALAUTHORIZE_H

#include "UniversalCmd.h"
#include <string>

class UniversalAuthorize : public UniversalCmd {
private:
    int volume{}, nozzle{}, price{}, dBtransactionId{};
    std::string user{}, productName{};
public:
    UniversalAuthorize() = default;
    UniversalAuthorize(int _columnId, CmdType _type, int _volume, int _nozzle, int _price,
                       std::string _user);
    ~UniversalAuthorize() override = default;
    int getNozzle() const;
    int getVolume() const;
    int getPrice() const;
    std::string getUser() const;
    int getDbTransactionId() const;
    void setUser(const std::string& _user);
    void setVolume(int _volume);
    void setNozzle(int _nozzle);
    void setPrice(int _price);
    void setDbTransactionId(int _id);
};


#endif //PTS2_0_UNIVERSALAUTHORIZE_H
