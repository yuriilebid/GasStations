#include "Protocols/ProtocolsMasters/ProtocolMaster.h"

double ProtocolMaster::getNumberValueOfHexArray(std::vector<uint8_t> arr, int posStart, int posEnd) {
    std::string str(arr.begin() + posStart, arr.begin() + posEnd);
//    str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
    double sumNumber = std::stod(str);
    return sumNumber;
}