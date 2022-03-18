#include "ProtocolSlaveShelf.h"

#include "utils/logger.h"

int getRequestPistol(int num) {
    return (num % 5);
}

int calcrc(std::vector<unsigned char> ptr, int count) {
    int b = 0;
    int crc;
    char i;
    crc = 0;
    while (--count >= 0) {
        crc = crc ^ (int) ptr[b++] << 8;
        i = 8;
        do {
            if (crc & 0x8000)
            crc = crc << 1 ^ 0x1021;
            else
            crc = crc << 1;
        } while(--i);
    }
    return (crc);
}

int getRequestTrkId(int num) {
    num -= 10;
    num /= 5;
    num += 1;
    return num;
}

std::vector<unsigned char> ProtocolSlaveShelf::responseHangedFree(const std::shared_ptr<UniversalState>& data) {
    std::vector<unsigned char> cmd;

    ///  2D F 8D 7 0 BC 1D
    cmd.push_back(0x2D);
    cmd.push_back(currentAddr);
    cmd.push_back(currentPacketId);
    cmd.push_back(0x09); /// LENGTH
    cmd.push_back(0x81); /// SLEEP MODE STATUS ANSWEARS

    /// Default nozzle up for the first nozzle
    cmd.push_back(0x01); /// Pistol status
    cmd.push_back(0x20); /// TRK status
     /// TMP above
    uint16_t crc = calcrc(cmd, 7);
    cmd.push_back(crc);
    cmd.push_back(crc >> 8);
    return cmd;
}

std::vector<unsigned char> ProtocolSlaveShelf::responseRemovedNozzle(const std::shared_ptr<UniversalState>& data) {
    std::vector<unsigned char> cmd;
    int nozzleRequested = data->getNozzle();

    ///  2D F 8D 7 0 BC 1D
    cmd.push_back(0x2D);
    cmd.push_back(currentAddr);
    cmd.push_back(currentPacketId);
    cmd.push_back(0x09); /// LENGTH
    cmd.push_back(0x81);
    cmd.push_back(0x01 | (1 << nozzleRequested));
    cmd.push_back(0x21); /// TRK status
    uint16_t crc = calcrc(cmd, 7);
    cmd.push_back(crc);
    cmd.push_back(crc >> 8);
//    LogPrintf(spdlog::level::debug, "|SHELF| Formed packet: {:X}", fmt::join(cmd, " "));
    return cmd;
}

std::vector<unsigned char> ProtocolSlaveShelf::responseAmountInfo(const std::shared_ptr<UniversalAmount>& data) {
    std::vector<unsigned char> cmd;
    int requestedNozzle = data->getNozzle();
    int volumeInMl = data->getVolume();

    volumeInMl /= 10; /// Need to convert from 1000 ml to 100 for Shelf

    cmd.push_back(0x2D);
    cmd.push_back(currentAddr);
    cmd.push_back(currentPacketId);
    cmd.push_back(0x0E);
    cmd.push_back(0x84);
    cmd.push_back(0x01 << requestedNozzle);
    cmd.push_back(0x8F);
    cmd.push_back(0x02);
    cmd.push_back(currentAddr);
    cmd.push_back(volumeInMl);
    cmd.push_back(volumeInMl >> 8);
    cmd.push_back(volumeInMl >> 16);
    uint16_t crc = calcrc(cmd, 12);
    cmd.push_back(crc);
    cmd.push_back(crc >> 8);

    LogPrintf(spdlog::level::info, "|SHELF| Responding |FUEL_SUPPLY| with |{}|ml.", volumeInMl * 10);

    return cmd;
}

std::vector<unsigned char> ProtocolSlaveShelf::responseSupplyDone(const std::shared_ptr<UniversalAmount>& data) {
    std::vector<unsigned char> cmd;
    int activeNozzle = data->getNozzle();
    /// TODO: TMP check
    int volumeInMl = data->getVolume() / 10.0;
    int sumInCent = data->getSum();
    int priceInCents = data->getPrice();

    cmd.push_back(0x2D);
    cmd.push_back(currentAddr);
    cmd.push_back(currentPacketId);
    cmd.push_back(0x11);
    cmd.push_back(0x93);
    cmd.push_back(0x01);
//    cmd.push_back(0x01 | activeNozzle << 1);
    cmd.push_back(0x81);
    cmd.push_back(volumeInMl);
    cmd.push_back(volumeInMl >> 8);
    cmd.push_back(volumeInMl >> 16);
    cmd.push_back(sumInCent);
    cmd.push_back(sumInCent >> 8);
    cmd.push_back(sumInCent >> 16);
    cmd.push_back(priceInCents);
    cmd.push_back(priceInCents >> 8);

    uint16_t crc = calcrc(cmd, 15);
    cmd.push_back(crc);
    cmd.push_back(crc >> 8);
    LogPrintf(spdlog::level::info, "|SHELF| Responding supply done with |{}|ml. Sum |{}|, Price |{}|", volumeInMl * 10, sumInCent, priceInCents);
    return cmd;
}

std::vector<unsigned char>
ProtocolSlaveShelf::responseClosedTransaciton(const std::shared_ptr<UniversalAmount>& data) {
    std::vector<unsigned char> cmd;
    int volumeInMl = data->getVolume();
    int priceInCents = data->getPrice();

    volumeInMl /= 10; /// Need to be converted from 1000 ml to 100 for Shelf

    cmd.push_back(0x2D);
    cmd.push_back(currentAddr);
    cmd.push_back(currentPacketId);
    cmd.push_back(0x16);
    cmd.push_back(0x85);
    cmd.push_back(0x02);
    cmd.push_back(0x81);
    cmd.push_back(0x01);
    cmd.push_back(currentAddr);
    cmd.push_back(volumeInMl);
    cmd.push_back(volumeInMl >> 8);
    cmd.push_back(volumeInMl >> 16);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    cmd.push_back(priceInCents);
    cmd.push_back(priceInCents >> 8);
    uint16_t crc = calcrc(cmd, 20);
    cmd.push_back(crc);
    cmd.push_back(crc >> 8);
    return cmd;
}

bool ProtocolSlaveShelf::checkCheckSum(const std::vector<unsigned char>& cmd) {
    std::vector<unsigned char> checkVecPacket;
    int sorucePacketSize = cmd.size();

    checkVecPacket.reserve(sorucePacketSize - 2);
    uint16_t crc = calcrc(cmd, sorucePacketSize - 2);
    unsigned char left = crc;
    unsigned char right = crc >> 8;

    if(left == cmd.at(sorucePacketSize - 2) and right == cmd.at(sorucePacketSize - 1)) {
        return true;
    } else {
        return false;
    }
}

std::vector<unsigned char> ProtocolSlaveShelf::responseConfirmation(const std::shared_ptr<UniversalState> &data) {
    std::vector<unsigned char> cmd;

    ///  2D F 8D 7 0 BC 1D
    cmd.push_back(0x2D);
    cmd.push_back(currentAddr);
    cmd.push_back(currentPacketId);
    cmd.push_back(0x07); /// LENGTH
    cmd.push_back(0x0C);
    uint16_t crc = calcrc(cmd, 5);
    cmd.push_back(crc);
    cmd.push_back(crc >> 8);
    return cmd;
}

std::vector<unsigned char> ProtocolSlaveShelf::responseTotaInfo(const std::shared_ptr<UniversalTotals>& data) {
    std::vector<uint8_t> cmd;
    int volumeInMl = data->getVolume();

    volumeInMl /= 10;

    cmd.push_back(0x2D);
    cmd.push_back(currentAddr);
    cmd.push_back(currentPacketId);
    cmd.push_back(0x0B); /// Length of the packet: 10;
    cmd.push_back(0xA0); /// Total Counters Request (TCR)
    cmd.push_back(volumeInMl);
    cmd.push_back(volumeInMl >> 8);
    cmd.push_back(volumeInMl >> 16);
    cmd.push_back(volumeInMl >> 24);
    uint16_t crcRes = calcrc(cmd, 9);
    cmd.push_back(crcRes);
    cmd.push_back(crcRes >> 8);
    return cmd;
}

std::vector<unsigned char> ProtocolSlaveShelf::preparePacketVec(std::shared_ptr<UniversalState> cmd, int addr) {
    auto statusType = cmd->getType();

    switch(statusType) {
        case StateType::HANGED_FREE:
            return responseHangedFree(cmd);
        case StateType::REMOVED_FREE:
        case StateType::REMOVED_WAITING:
            return responseRemovedNozzle(cmd);
        case StateType::FUEL_SUPPLY:
            LogPrintf(spdlog::level::info, "Responding |FUEL_SUPPLY| to |FISCAL|");
            return responseAmountInfo(std::static_pointer_cast<UniversalAmount>(cmd));
        case StateType::SUPPLY_DONE:
            LogPrintf(spdlog::level::info, "Responding |SUPPLY_DONE| to |FISCAL|");
            return responseSupplyDone(std::static_pointer_cast<UniversalAmount>(cmd));
        case StateType::TRANSACTION_ClOSED:
            LogPrintf(spdlog::level::info, "Responding |TRANSACTION_ClOSED| to |FISCAL|");
            return responseClosedTransaciton(std::static_pointer_cast<UniversalAmount>(cmd));
        case StateType::SUPPLY_STOPPED:
            LogPrintf(spdlog::level::info, "Responding |TRANSACTION_STOPED| to |FISCAL|");
            return responseConfirmation(cmd);
        case StateType::TOTAL_RESPONSE:
            LogPrintf(spdlog::level::info, "Responding |TOTAL_RESPONSE| to |FISCAL|");
            return responseTotaInfo(std::static_pointer_cast<UniversalTotals>(cmd));
        default:
            LogPrintf(spdlog::level::err, "Responding |UNDEFINED| type to |FISCAL|");
            return responseHangedFree(cmd);
    }
}

std::unique_ptr<UniversalAuthorize> ProtocolSlaveShelf::resolveWriteVolumeRequest(const std::vector<unsigned char> &cmd) {
    auto volume = 0, price = 0;
    uint8_t dis1, dis2;
    auto address = getRequestTrkId(cmd.at(1));
    auto nozzle = getRequestPistol(cmd.at(1)) + 1;

    dis1 = cmd.at(5);
    dis2 = cmd.at(6);

    volume |= cmd.at(7);
    volume |= cmd.at(8) << 8;
    volume |= cmd.at(9) << 16;

    price |= cmd.at(10);
    price |= cmd.at(11) << 8;

    volume *= 10; /// We use ML

    LogPrintf(spdlog::level::info, "|FISCAL| Volume request on |{}|ml. price |{}| pump |{}| nozzle |{}|", volume, price, address, nozzle);
    /// UniversalAuthorize::UniversalAuthorize(int _columnId, CmdType _type, int _volume, int _nozzle, int _price) {
    return std::make_unique<UniversalAuthorize>(address, CmdType::WRITE_VOLUME, volume, nozzle, price, "FISCAL");
}

std::vector<unsigned char> ProtocolSlaveShelf::traceResponse(std::vector<unsigned char> packet) {
    int addr = packet.at(1);
    char colourFullPacket[512];

    bzero(colourFullPacket, 512);

    if (lastReceivedPacket.find(addr) != lastReceivedPacket.end()) {
        auto oldSize = lastReceivedPacket[addr].size();
        auto newSize = packet.size();

        lastReceivedPacket[addr][2] = packet[2];
        lastReceivedPacket[addr][oldSize - 1] = packet[newSize - 1];
        lastReceivedPacket[addr][oldSize - 2] = packet[newSize - 2];
        if (lastReceivedPacket[addr] != packet) {
            auto oldPacket = lastReceivedPacket[addr];

            for (auto i = 0; i < newSize; i++) {
                if (i >= oldSize) {
                    if (i < newSize - 2) {
                        sprintf(colourFullPacket, "%s \033[1;32m%X\033[0m", colourFullPacket, packet.at(i));
                    } else {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    }
                } else {
                    if (packet[i] == oldPacket[i]) {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    } else {
                        if(i < newSize - 2) {
                            sprintf(colourFullPacket, "%s \033[1;32m%X\033[0m", colourFullPacket, packet.at(i));
                        } else {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        }
                    }
                }
            }
            LogConsole(spdlog::level::trace, "|TCP||READ|{}", colourFullPacket);
            LogFile(spdlog::level::trace, "|TCP||READ|: {:X}", fmt::join(packet, " "));
        }
    }
    lastReceivedPacket[addr] = packet;
    return packet;
}

std::vector<unsigned char> ProtocolSlaveShelf::traceRequests(std::vector<unsigned char> packet) {
    int addr = packet.at(1);
    char colourFullPacket[512];

    bzero(colourFullPacket, 512);

    if (lastSentPacket.find(addr) != lastSentPacket.end()) {
        auto oldSize = lastSentPacket[addr].size();
        auto newSize = packet.size();

        lastSentPacket[addr][2] = packet[2];
        lastSentPacket[addr][oldSize - 1] = packet[newSize - 1];
        lastSentPacket[addr][oldSize - 2] = packet[newSize - 2];
        if (lastSentPacket[addr] != packet) {
            auto oldPacket = lastSentPacket[addr];

            for (auto i = 0; i < newSize; i++) {
                if (i >= oldSize) {
                    if (i < newSize - 2) {
                        sprintf(colourFullPacket, "%s \033[1;34m%X\033[0m", colourFullPacket, packet.at(i));
                    } else {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    }
                } else {
                    if (packet[i] == oldPacket[i]) {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    } else {
                        if(i < newSize - 2) {
                            sprintf(colourFullPacket, "%s \033[1;34m%X\033[0m", colourFullPacket, packet.at(i));
                        } else {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        }
                    }
                }
            }
            LogConsole(spdlog::level::trace, "|TCP||WRITE|{}", colourFullPacket);
            LogFile(spdlog::level::trace, "|TCP||WRITE|: {:X}", fmt::join(packet, " "));
        }
    } else {
//        LogPrintf(spdlog::level::debug, "|TCP||WRITE||NEW_ADDRESS|: {:X}", fmt::join(packet, " "));
    }
    lastSentPacket[addr] = packet;
    return packet;
}

UniversalSetPrice ProtocolSlaveShelf::resolveSetPriceRequest(const std::vector<unsigned char> &cmd) {
    auto price = 0;
    auto address = cmd.at(1);

    price |= cmd.at(7);
    price |= cmd.at(8) << 8;
    price |= cmd.at(9) << 16;
    return {address, CmdType::WRITE_PRICE, price};
}

std::unique_ptr<UniversalCmd> ProtocolSlaveShelf::parseCmd(const std::vector<unsigned char>& cmd) {
    std::unique_ptr<UniversalCmd> resCmd;
    unsigned char sourceAddress = cmd.at(1);
    currentPacketId = cmd.at(2);
    currentAddr = sourceAddress;
    int address = getRequestTrkId(sourceAddress);
    CmdType type;

//    LogPrintf(spdlog::level::info, "|SHELF| Parsing packet: {:X} getting index: {}", fmt::join(cmd, " "), cmdIndex);

    switch (cmd.at(cmdIndex)) {
        case StatusRequest:
            resCmd = std::make_unique<UniversalCmd>(UniversalCmd(address, CmdType::STATUS_REQUEST));
            break;
        case AmountInfo:
            resCmd = std::make_unique<UniversalCmd>(UniversalCmd(address, CmdType::STATUS_REQUEST));
            LogPrintf(spdlog::level::info, "|CMD| |AMOUNT_INFO| from |FISCAL| to |SOFTPTS|");
            break;
        case ReadPrice:
            resCmd = std::make_unique<UniversalCmd>(UniversalCmd(address, CmdType::READ_PRICE));
            LogPrintf(spdlog::level::info, "|CMD| |READ_PRICE| from |FISCAL| to |SOFTPTS|");
            break;
        case WritePrice:
            resCmd = std::make_unique<UniversalCmd>(resolveSetPriceRequest(cmd));
            LogPrintf(spdlog::level::info, "|CMD| |WRITE_PRICE| from |FISCAL| to |SOFTPTS|");
            break;
        case WriteVolume:
            resCmd = resolveWriteVolumeRequest(cmd);
            LogPrintf(spdlog::level::info, "|CMD| |WRITE_VOLUME| from |FISCAL| to |SOFTPTS|");
            break;
        case StopCommand:
            resCmd = std::make_unique<UniversalCmd>(UniversalCmd(address, CmdType::STOP_COMMAND));
            LogPrintf(spdlog::level::info, "|CMD| |STOP_COMMAND| from |FISCAL| to |SOFTPTS|");
            break;
        case WriteMoney:
            type = CmdType::WRITE_MONEY;
            break;
        case WriteFullTank:
            type = CmdType::WRITE_VOLUME;
            break;
        case TotalCountersRequest:
        case CurrentCountersRequest:
            resCmd = std::make_unique<UniversalCmd>(UniversalCmd(address, CmdType::GET_TOTAL));
            LogPrintf(spdlog::level::info, "|CMD| |TOTAL_COUNTERS| from |FISCAL| to |SOFTPTS|");
            break;
        default:
            resCmd = std::make_unique<UniversalCmd>(UniversalCmd(address, CmdType::GET_TOTAL));
            LogPrintf(spdlog::level::warn, "|CMD| |UNDEFINED| from |FISCAL| to |SOFTPTS| code |{}|", cmd.at(cmdIndex));
            break;
    }
    return resCmd;
}