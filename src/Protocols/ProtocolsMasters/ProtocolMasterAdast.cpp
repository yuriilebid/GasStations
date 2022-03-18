#include "UniversalAuthorize.h"
#include "UniversalAmount.h"
#include "ProtocolMasterAdast.h"
#include "logger.h"
#include <cmath>

unsigned int ProtocolMasterAdast::calc_crc(const std::vector<unsigned char> &cmd) {
    int current_crc_value = 0;

    for (unsigned char i : cmd) {
        current_crc_value ^= i & 0xFF;
        for (int j = 0; j < 8; j++) {
            if ((current_crc_value & 1) != 0) {
                current_crc_value = (current_crc_value >> 1) ^ 0x8408; /// Polynomial
            } else {
                current_crc_value = current_crc_value >> 1;
            }
        }
    }
    return current_crc_value & 0xFFFF;
}

std::vector<unsigned char> ProtocolMasterAdast::closeCommand(std::unique_ptr<UniversalClose> cmd, int addr) {
    std::vector<uint8_t> packet;
    std::vector<uint8_t> dataCmd;

    /// Sync words (2 bytes)
    packet.push_back(0x16);
    packet.push_back(0x16);
    /// Address (1 byte)
    dataCmd.push_back(addr);
    packet.push_back(addr);
    /// Length (3 byte)
    packet.push_back(0x03);
    dataCmd.push_back(0x03);
    /// Command (1 byte)
    packet.push_back(0x4C); /// M_FORGET
    dataCmd.push_back(0x4C);
    /// DATA (3 bytes)
    dataCmd.push_back(pumpIds.at(addr) >> 8);
    dataCmd.push_back(pumpIds.at(addr));
    packet.push_back(pumpIds.at(addr) >> 8);
    packet.push_back(pumpIds.at(addr));
    uint16_t crcNum = calc_crc(dataCmd);
    packet.push_back(crcNum);
    packet.push_back(crcNum >> 8);
    packet.push_back(0xFF);
    return packet;
}

std::vector<unsigned char> ProtocolMasterAdast::getLastPacket(int addr) {
    if (lastReceived51Packet.find(addr) != lastReceived51Packet.end()) {
        return lastReceived51Packet[addr];
    } else {
        return {};
    }
}

std::vector<unsigned char> ProtocolMasterAdast::traceResponse(std::vector<unsigned char> packet) {
    int addr = packet.at(2);
    char colourFullPacket[512];

    bzero(colourFullPacket, 512);

    /// lastReceived51Packet
    if (packet[cmdIndex] == 0x4A) {
        if (lastReceived51Packet.find(addr) != lastReceived51Packet.end()) {
            if (lastReceived51Packet[addr] != packet) {
                auto oldPacket = lastReceived51Packet[addr];
                auto oldSize = oldPacket.size();
                auto newSize = packet.size();

                for (auto i = 0; i < newSize; i++) {
                    if (i >= oldSize) {
                        if (i < newSize - 3) {
                            sprintf(colourFullPacket, "%s \033[0;32m%X\033[0m", colourFullPacket, packet.at(i));
                        } else {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        }
                    } else {
                        if (packet[i] == oldPacket[i]) {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        } else {
                            if (i < newSize - 3) {
                                sprintf(colourFullPacket, "%s \033[0;32m%X\033[0m", colourFullPacket, packet.at(i));
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
        lastReceived51Packet[addr] = packet;
    } else {
        if (lastReceivedPacket.find(addr) != lastReceivedPacket.end()) {
            if (lastReceivedPacket[addr] != packet) {
                auto oldPacket = lastReceivedPacket[addr];
                auto oldSize = oldPacket.size();
                auto newSize = packet.size();

                for (auto i = 0; i < newSize; i++) {
                    if (i >= oldSize) {
                        if (i < newSize - 3) {
                            sprintf(colourFullPacket, "%s \033[0;32m%X\033[0m", colourFullPacket, packet.at(i));
                        } else {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        }
                    } else {
                        if (packet[i] == oldPacket[i]) {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        } else {
                            if (i < newSize - 3) {
                                sprintf(colourFullPacket, "%s \033[0;32m%X\033[0m", colourFullPacket, packet.at(i));
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
    }
    return packet;
}

std::vector<unsigned char> ProtocolMasterAdast::traceRequests(std::vector<unsigned char> packet) {
    int addr = packet.at(2);
    char colourFullPacket[512];

    bzero(colourFullPacket, 512);

    if (lastSentPacket.find(addr) != lastSentPacket.end()) {
        if (lastSentPacket[addr] != packet) {
            auto oldPacket = lastSentPacket[addr];
            auto oldSize = oldPacket.size();
            auto newSize = packet.size();

            for (auto i = 0; i < newSize; i++) {
                if (i >= oldSize) {
                    if (i < newSize - 3) {
                        sprintf(colourFullPacket, "%s \033[0;34m%X\033[0m", colourFullPacket, packet.at(i));
                    } else {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    }
                } else {
                    if (packet[i] == oldPacket[i]) {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    } else {
                        if(i < newSize - 3) {
                            sprintf(colourFullPacket, "%s \033[0;34m%X\033[0m", colourFullPacket, packet.at(i));
                        } else {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        }
                    }
                }
            }
            LogConsole(spdlog::level::trace, "|TCP||WRITE|{}", colourFullPacket);
            LogFile(spdlog::level::trace, "|TCP||WRITE|: {:X}", fmt::join(packet, " "));
        }
    }
    lastSentPacket[addr] = packet;
    return packet;
}

std::vector<unsigned char> ProtocolMasterAdast::preparePacket(std::unique_ptr<UniversalCmd> cmd, int addr) {
    std::vector<unsigned char> packet {};

    switch(cmd->getCmdType()) {
        case CmdType::STATUS_REQUEST:
            packet = statusRequest(std::move(cmd), addr);
            break;
        case CmdType::WRITE_VOLUME:
            packet = runCommand(std::move(std::make_unique<UniversalAuthorize>(*dynamic_cast<UniversalAuthorize*>(cmd.release()))), addr);
            break;
        case CmdType::STOP_COMMAND:
            packet = stopCommand(std::move(cmd), addr);
            break;
        case CmdType::GET_TOTAL:
            packet = totalCommand(std::move(std::make_unique<UniversalTotal>(*dynamic_cast<UniversalTotal*>(cmd.release()))), addr);
            break;
        case CmdType::WRITE_MONEY:
            packet = setPriceRequest(std::move(std::make_unique<UniversalAuthorize>(*dynamic_cast<UniversalAuthorize*>(cmd.release()))), addr);
            break;
        case CmdType::CLOSE_REPORT:
            packet = closeCommand(std::move(std::make_unique<UniversalClose>(*dynamic_cast<UniversalClose*>(cmd.release()))), addr);
            break;
        default:
            LogPrintf(spdlog::level::warn, "Cannot form packet, undefined command. Code: {}", cmd->getCmdType());
            break;
    }
    return packet;
}

std::vector<unsigned char> ProtocolMasterAdast::runCommand(std::unique_ptr<UniversalAuthorize> cmd, int addr) {
    std::vector<unsigned char> packet;
    std::vector<unsigned char> crccmd;
    int volume = cmd->getVolume() / 10;
    int price = cmd->getPrice();

    lastPriceAuthorize[addr] = price;

    /// Sync words (2 bytes)
    packet.push_back(0x16);
    packet.push_back(0x16);
    /// Slave Address Word (1 byte)
    packet.push_back(addr);
    crccmd.push_back(addr);
    /// Length Word (1 byte)
    packet.push_back(0x0E); /// 14 bytes
    crccmd.push_back(0x0E);
    /// Data words (up to 43 bytes)
    packet.push_back(0x39); /// Command G_Enable
    crccmd.push_back(0x39);
    packet.push_back(0x30 + cmd->getNozzle());
    crccmd.push_back(0x30 + cmd->getNozzle());
    packet.push_back('V'); /// preselect transaction enabled (requestedVolume),
    crccmd.push_back('V');
    for(int i = 0; i < 6; i++) {
        int digitVolume = volume / static_cast<int>(pow(10, 5 - i));
        crccmd.push_back(digitVolume + 0x30);
        packet.push_back(digitVolume + 0x30);
        volume -= digitVolume * static_cast<int>(pow(10, 5 - i));
    }
    for(int j = 0; j < cmd->getNozzle(); j++) {
        packet.push_back(0x31);
        crccmd.push_back(0x31);
        for (int i = 0; i < 4; i++) {
            int digitVolume = price / static_cast<int>(pow(10, 3 - i));
            crccmd.push_back(digitVolume + 0x30);
            packet.push_back(digitVolume + 0x30);
            price -= digitVolume * static_cast<int>(pow(10, 3 - i));
        }
    }
    /// SRC Word (2 bytes)
    auto crc = calc_crc(crccmd);
	packet.push_back(crc); /// CRC16
	packet.push_back(crc >> 8); /// CRC16
	/// Trailing PAD (1 byte)
	packet.push_back(0xFF);
	return packet;
}

/// Service station console sends the command at any time when
/// it does not need to send any runCommand other command (used as the poll command).
std::vector<unsigned char> ProtocolMasterAdast::statusRequest(std::unique_ptr<UniversalCmd> lastTransaction, int addr) {
    std::vector<unsigned char> cmd;
    std::vector<unsigned char> dataCmd;

    cmd.push_back(0x16); /// DATA SYNC
    cmd.push_back(0x16); /// DATA SYNC
    cmd.push_back(addr);
    dataCmd.push_back(addr);
    cmd.push_back(0x01); /// Data length 1 as no Data
    dataCmd.push_back(0x01);
    cmd.push_back(0x4A); /// Status
    dataCmd.push_back(0x4A);

    auto crc = calc_crc(dataCmd);
	cmd.push_back(crc); /// CRC16
	cmd.push_back(crc >> 8); /// CRC16
	cmd.push_back(0xFF);

    return cmd;
}

std::vector<unsigned char> ProtocolMasterAdast::stopCommand(std::unique_ptr<UniversalCmd> cmd, int addr) {
    std::vector<unsigned char> packet;
    std::vector<unsigned char> crcCmd;

    packet.push_back(0x16);
    packet.push_back(0x16);
    packet.push_back(addr);
    crcCmd.push_back(addr);
    packet.push_back(0x01); /// Length
    crcCmd.push_back(0x01);
    packet.push_back(0x35);
    crcCmd.push_back(0x35);
    unsigned short crcCalculated = calc_crc(crcCmd);
    packet.push_back(crcCalculated); /// CRC16
    packet.push_back(crcCalculated >> 8); /// CRC16
    packet.push_back(0xff);

    return packet;
}

std::vector<unsigned char> ProtocolMasterAdast::totalCommand(std::unique_ptr<UniversalTotal> cmd, int addr) {
    std::vector<uint8_t> packet;
    std::vector<uint8_t> crccmd;

//    LogPrintf(spdlog::level::debug, "|PTS| Creating source in |handTotalRequest|");
    /// Sync words (2 bytes)
    packet.push_back(0x16);
    packet.push_back(0x16);
    /// Slave Address Word (1 byte)
    packet.push_back(addr);
    crccmd.push_back(addr);
    /// Length Word (1 byte)
    packet.push_back(0x02); /// 14 bytes
    crccmd.push_back(0x02);

    packet.push_back(0x42); /// 14 bytes
    crccmd.push_back(0x42);
    /// Data words (up to 43 bytes)
//    cmd.push_back(0x63); /// Command G_Enable
//    crccmd.push_back(0x63);
    packet.push_back(0x30 + cmd->getNozzle());
    crccmd.push_back(0x30 + cmd->getNozzle());
    /// SRC Word (2 bytes)
    auto crc = calc_crc(crccmd);
	packet.push_back(crc); /// CRC16
	packet.push_back(crc >> 8); /// CRC16
	/// Trailing PAD (1 byte)
	packet.push_back(0xFF);
	return packet;
}

std::vector<unsigned char> ProtocolMasterAdast::setPriceRequest(std::unique_ptr<UniversalAuthorize> cmd, int addr) {
    std::vector<unsigned char> packet;
    std::vector<unsigned char> crccmd;
    int price = cmd->getPrice();

    /// Sync words (2 bytes)
    packet.push_back(0x16);
    packet.push_back(0x16);
    /// Slave Address Word (1 byte)
    packet.push_back(addr);
    crccmd.push_back(addr);
    /// Length Word (1 byte)
    packet.push_back(0x06);
    crccmd.push_back(0x06);
    /// TODO: Multiple nozzles response
    packet.push_back(0x34); /// 14 bytes
    crccmd.push_back(0x34);
    /// G_Setprices command
    packet.push_back(0x01);
    crccmd.push_back(0x01);
    for (int i = 0; i < 4; i++) {
        int digitVolume = price / static_cast<int>(pow(10, 3 - i));
        crccmd.push_back(digitVolume + 0x30);
        packet.push_back(digitVolume + 0x30);
        price -= digitVolume * static_cast<int>(pow(10, 3 - i));
    }
    auto crc = calc_crc(crccmd);
	packet.push_back(crc); /// CRC16
	packet.push_back(crc >> 8); /// CRC16
	/// Trailing PAD (1 byte)
	packet.push_back(0xFF);
    return packet;
}

std::shared_ptr<UniversalState> ProtocolMasterAdast::parseAmountInfo(const std::vector<unsigned char>& cmd) {
    int address = cmd.at(2);
    int newTxnId = cmd.at(5) << 8;
    newTxnId = newTxnId | cmd.at(6);
    pumpIds[address] = newTxnId;
    int activeNozzle = cmd.at(7) - 0x30;
    StateType type = StateType::FUEL_SUPPLY;
    int volumeInMl = static_cast<int>((getNumberValueOfHexArray(cmd, 17, 24) + 0.005) * 1000.0);
    int sumInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 10, 17) + 0.005) * 100.0);
    int priceInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 24, 29) + 0.005) * 100.0);

    /// Костыль
    if(lastPriceAuthorize.find(address) != lastPriceAuthorize.end()) {
        priceInCents = lastPriceAuthorize[address];
        sumInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 17, 24) + 0.005) * static_cast<float>(priceInCents));
    }

    auto resAmount = std::make_shared<UniversalAmount>(UniversalAmount(address, activeNozzle, 0, volumeInMl,
                                                                               priceInCents, newTxnId, type, sumInCents));
    return resAmount;
}

std::shared_ptr<UniversalState> ProtocolMasterAdast::parseEndDispensingInfo(const std::vector<unsigned char>& cmd) {
    std::shared_ptr<UniversalState> resSample;
    StateType type = StateType::SUPPLY_DONE;
    int address = cmd.at(2);
    int newTxnId = cmd.at(5) << 8;
    newTxnId = newTxnId | cmd.at(6);
    pumpIds[address] = newTxnId;
    int activeNozzle = cmd.at(7) - 0x30;
    int productCode = cmd.at(8) - 0x30;
    int volumeInMl = static_cast<int>((getNumberValueOfHexArray(cmd, 17, 24) + 0.005) * 1000.0);
    int sumInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 10, 17) + 0.005) * 100.0);
    int priceInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 24, 29) + 0.005) * 100.0);

    /// Костыль
    if(lastPriceAuthorize.find(address) != lastPriceAuthorize.end()) {
        priceInCents = lastPriceAuthorize[address];
        sumInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 17, 24) + 0.005) * static_cast<float>(priceInCents));
    }

    auto resAmount = std::make_shared<UniversalAmount>(UniversalAmount(address, activeNozzle, productCode, volumeInMl,
                                                                               priceInCents, newTxnId, type, sumInCents));
    return resAmount;
}

std::shared_ptr<UniversalState> ProtocolMasterAdast::parseTotalsInfo(const std::vector<unsigned char> &cmd, int id) {
    int activeNozzle = cmd.at(5) - 0x30;
    int address = cmd.at(2);
    long long volumeInMl = static_cast<long long>((getNumberValueOfHexArray(cmd, 6, 19) + 0.005) * 1000.0);
//    int sumInCents = getNumberValueOfHexArray(cmd, 10, 17);
    int priceInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 19, 32) + 0.005) * 100.0);

    auto resAmount = std::make_shared<UniversalTotals>(UniversalTotals(volumeInMl, priceInCents));
    resAmount->setId(id);
    resAmount->setNozzle(activeNozzle);
    resAmount->setType(StateType::TOTAL_RESPONSE);
    return resAmount;
}

std::shared_ptr<UniversalState> ProtocolMasterAdast::parseRemovedInfo(const std::vector<unsigned char>& cmd) {
    int activeNozzle = cmd.at(5);
    int address = cmd.at(2);

    return std::make_shared<UniversalState>(UniversalState(address, activeNozzle, StateType::ERROR, 0));
}

std::shared_ptr<UniversalState> ProtocolMasterAdast::parseHangedFreeInfo(const std::vector<unsigned char>& cmd) {
    int address = cmd.at(2);
    int packetSize = cmd.size();

    for (int i = 5; i < 11; i++) {
        if(i < packetSize) {
            if ((cmd.at(i) - 0x30) != 0) {
                switch (cmd.at(i) - 0x30) {
                    case 0:
                    case 1:  /// Nozzle down
                        break;
                    case 2:
                    case 3: {
                        int activeNozzle = i - 4;
                        return std::make_shared<UniversalState>(
                                UniversalState(address, activeNozzle, StateType::REMOVED_WAITING, 0));
                    }
                    /// nozzle down after the E-type error transaction end
                    case 6:
                         return std::make_shared<UniversalState>(
                                UniversalState(address, 0, StateType::HANGED_FREE, 0));
                    case 8:
                    case 0x0A:
                    case 0x0D:
                    case 0x11: {
                        int activeNozzle = i - 4;
                        return std::make_shared<UniversalState>(
                                UniversalState(address, activeNozzle, StateType::HANGED_FREE, 0));
                    }
                    default:
                        LogPrintf(spdlog::level::warn, "|NOTE| Responding StateType::ERROR because state of nozzle is: {}", cmd.at(i) - 0x30);
                        return std::make_shared<UniversalState>(UniversalState(address, 0, StateType::ERROR, 0));
                }
            }
        }
    }
    return std::make_shared<UniversalState>(UniversalState(address, 0, StateType::HANGED_FREE, 0));
}

std::shared_ptr<UniversalAuthorizeConfirmation> ProtocolMasterAdast::resolveActiveKeyboard(const std::vector<unsigned char>& cmd) {
    LogPrintf(spdlog::level::info, "|TRK| Active keyboard active");
    int address = cmd.at(2);
    int activeNozzle = cmd.at(5); /// TODO: check this info
    int volumeInMl = static_cast<int>((getNumberValueOfHexArray(cmd, 6, 12) + 0.005) * 1000.0);
    int priceInCents = static_cast<int>((getNumberValueOfHexArray(cmd, 12 + ((activeNozzle - 1) * 5) + 1, 16) + 0.005) * 100.0);

    auto data = std::make_shared<UniversalAuthorizeConfirmation>(address, volumeInMl, StateType::AUTHORIZE_REGISTERED);
    data->setPrice(priceInCents);
    data->setNozzle(activeNozzle);
    return data;
}

std::shared_ptr<UniversalState>
ProtocolMasterAdast::parseResponse(const std::vector<unsigned char> &cmd, int expectedPumpAddr, int pumpId) {
    std::vector<uint8_t> emptyPacketResponse = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    switch (cmd[cmdIndex]) {
        case 0x39: {
            auto res = resolveActiveKeyboard(cmd);
            if(res->getId() == expectedPumpAddr) {
                res->setId(pumpId);
            } else {
                res->setType(StateType::ERROR);
            }
            return res;
        }
        case 0x31: {
            LogFile(spdlog::level::info, "|TRK| Responses with G_ENABLE command from keyboard");
        }
        case 0x52: {  /// AMOUNT_INFO
            auto res = parseAmountInfo(cmd);
            if(res->getId() == expectedPumpAddr) {
                res->setId(pumpId);
            } else {
                res->setType(StateType::ERROR);
            }
            return res;
        }
        case 0x61: {  /// END_OF_DISPENSING
            LogPrintf(spdlog::level::info, "|END_OF_DISPENSING| packet: {:X}", fmt::join(cmd, " "));
            auto res = parseEndDispensingInfo(cmd);
            if(res->getId() == expectedPumpAddr) {
                res->setId(pumpId);
            } else {
                res->setType(StateType::ERROR);
            }
            return res;
        }
        case 0x62: {  /// G_WR_NOZZLE_TOTS
            auto res = parseTotalsInfo(cmd, 0);
            if(res->getId() == expectedPumpAddr) {
                res->setId(pumpId);
            } else {
                res->setType(StateType::ERROR);
            }
            return res;
        }
        case 0x51: {  /// A_TRANS_ENABLE_RESPONSE. In general it means that pistol is up
            auto res = parseRemovedInfo(cmd);
            if(res->getId() == expectedPumpAddr) {
                res->setId(pumpId);
            } else {
                res->setType(StateType::ERROR);
            }
            return res;
        }
        case 0x4A: {
            auto res = parseHangedFreeInfo(cmd);
            if(res->getId() == expectedPumpAddr) {
                res->setId(pumpId);
            } else {
                res->setType(StateType::ERROR);
            }
            return res;
        }
        case 0x42: {
            LogPrintf(spdlog::level::info, "|ADAST||TOTAL| Response: {:X}", fmt::join(cmd, " "));
            auto res = parseTotalsInfo(cmd, pumpId);
            res->setId(pumpId);
            return res;
        }
        default:
            LogPrintf(spdlog::level::warn, "|TRK| Undefined commnad response: {:X}", fmt::join(cmd, " "));
            return std::make_shared<UniversalState>(UniversalState(pumpId, 0, StateType::ERROR, 0));
    }
}

bool ProtocolMasterAdast::checkCheckSum(const std::vector<unsigned char> &cmd) {
    std::vector<unsigned char> checkVector;
    int sourcePacketLength = cmd.size();

    if(sourcePacketLength < 6) {
        LogPrintf(spdlog::level::warn, "|TCP||ADAST| Small packet: {:X}", fmt::join(cmd, " "));
        return false;
    }

    for(int i = 2; i < sourcePacketLength - 3; i++) {
        checkVector.push_back(cmd.at(i));
    }
    auto rescrc = calc_crc(checkVector);
    unsigned char left = rescrc;
    unsigned char right = rescrc >> 8;

    if(right == cmd.at(sourcePacketLength - 2) and left == cmd.at(sourcePacketLength - 3)) {
        return true;
    } else {
        LogPrintf(spdlog::level::warn, "|TCP||ADAST| Problematic packet: {:X}", fmt::join(cmd, " "));
        return false;
    }
}

bool ProtocolMasterAdast::checkPacketAppropriation(const std::vector<unsigned char> &cmd) {
    if(cmd.size() < 6) {
        return false;
    }
    return true;
}
