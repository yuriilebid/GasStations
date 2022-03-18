#include "ProtocolMasterUnipump.h"
#include <cmath>
#include "UniversalAmount.h"
#include "UniversalTotals.h"
#include "logger.h"

typedef unsigned short WORD;
typedef unsigned char byte;

#define MAKE_TABS	0	/* Builds tables below */
#define FAST_CRC	1	/* If fast CRC should be used */

#if FAST_CRC & !MAKE_TABS

static WORD	crc_16_tab[] = {
	0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
	0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
	0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
	0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
	0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
	0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
	0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
	0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
	0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
	0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
	0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
	0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
	0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
	0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
	0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
	0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
	0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
	0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
	0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
	0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
	0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
	0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
	0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
	0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
	0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
	0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
	0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
	0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
	0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
	0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
	0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
	0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};
#endif


/* CRC-16 is based on the polynomial x^16 + x^15 + x^2 + 1.  Bits are */
/* sent LSB to MSB. */
WORD getCrc16( WORD crc, const std::vector<unsigned char>& buf) {
#if !(FAST_CRC & !MAKE_TABS)
	register int	i;
	register int	ch;
#endif

	for (auto& c : buf) {
#if FAST_CRC & !MAKE_TABS
		crc = (crc >> 8) ^ crc_16_tab[ (crc ^ c) & 0xff ];
#else
		ch = c
		for (i = 0; i < 8; i++) {
			if ((crc ^ ch) & 1)
				crc = (crc >> 1) ^ 0xa001;
			else
				crc >>= 1;
			ch >>= 1;
		}
#endif
	}
	return crc;
}

std::vector<unsigned char> ProtocolMasterUnipump::traceRequests(std::vector<unsigned char> packet) {
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
                    if (i < newSize - 4) {
                        sprintf(colourFullPacket, "%s \033[0;34m%X\033[0m", colourFullPacket, packet.at(i));
                    } else {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    }
                } else {
                    if (packet[i] == oldPacket[i]) {
                        sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                    } else {
                        if(i < newSize - 4) {
                            sprintf(colourFullPacket, "%s \033[0;34m%X\033[0m", colourFullPacket, packet.at(i));
                        } else {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        }
                    }
                }
            }
            LogConsole(spdlog::level::trace, "|TCP||WRITE|{}", colourFullPacket);
            LogFile(spdlog::level::trace, "|TCP||WRITE|{:X}", fmt::join(packet, " "));
        }
    }
    lastSentPacket[addr] = packet;
    return packet;
}

std::vector<unsigned char> ProtocolMasterUnipump::traceResponse(std::vector<unsigned char> packet) {
    try {
        int addr = packet.at(2);
        char colourFullPacket[512];

        bzero(colourFullPacket, 512);

        if (lastReceivedPacket.find(addr) != lastReceivedPacket.end()) {
            if (lastReceivedPacket[addr] != packet) {
                auto oldPacket = lastReceivedPacket[addr];
                auto oldSize = oldPacket.size();
                auto newSize = packet.size();

                for (auto i = 0; i < newSize; i++) {
                    if (i >= oldSize) {
                        if (i < newSize - 4) {
                            sprintf(colourFullPacket, "%s \033[0;32m%X\033[0m", colourFullPacket, packet.at(i));
                        } else {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        }
                    } else {
                        if (packet[i] == oldPacket[i]) {
                            sprintf(colourFullPacket, "%s %X", colourFullPacket, packet[i]);
                        } else {
                            if (i < newSize - 4) {
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
    } catch(std::out_of_range &e) {
        LogPrintf(spdlog::level::warn, "Packet: {:X}. Packet size: {}. Tracing log error: {}", fmt::join(packet, " "), packet.size(), e.what());
    }
    return packet;
}

std::vector<unsigned char> ProtocolMasterUnipump::preparePacket(std::unique_ptr<UniversalCmd> cmd, int addr) {
    std::vector<unsigned char> packet;

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
            packet = setPriceRequest(std::move(cmd), addr);
            break;
        case CmdType::CLOSE_REPORT:
            packet = closeCommand(
                    std::move(std::make_unique<UniversalClose>(*dynamic_cast<UniversalClose *>(cmd.release()))), addr);
            break;
        default:
            // TODO: check what should it be here
            break;
    }
    return packet;
}

std::vector<unsigned char> ProtocolMasterUnipump::totalCommand(std::unique_ptr<UniversalTotal> cmd, int addr) {
    std::vector<unsigned char> packet;
    std::vector<unsigned char> crccmd;

    packet.push_back(DLE);
    packet.push_back(STX);
    packet.push_back(addr + 0x30);
    crccmd.push_back(addr + 0x30);
    packet.push_back(0x54);
    crccmd.push_back(0x54);
    packet.push_back(0x30 + cmd->getNozzle());
    crccmd.push_back(0x30 + cmd->getNozzle());
    auto rescrc = calc_crc(crccmd);
    uint8_t leftPart = rescrc;
    uint8_t rightPart = rescrc >> 8;
    packet.push_back(leftPart);
    packet.push_back(rightPart);
    packet.push_back(DLE);
    packet.push_back(ETX);
    return packet;
}

std::vector<unsigned char> ProtocolMasterUnipump::statusRequest(std::unique_ptr<UniversalCmd> cmd, int addr) {
    std::vector<unsigned char> crcarr;
    std::vector<unsigned char> packet;

    packet.push_back(DLE);
    packet.push_back(STX);
    packet.push_back(addr + 0x30);
    packet.push_back(0x53); /// Code for status

    crcarr.push_back(addr + 0x30);
    crcarr.push_back(0x53);
    auto rescrc = calc_crc(crcarr);
    uint8_t leftPart = rescrc;
    uint8_t rightPart = rescrc >> 8;
    packet.push_back(leftPart);
    packet.push_back(rightPart);
    packet.push_back(DLE);
    packet.push_back(ETX);

    return packet;
}

std::vector<unsigned char> ProtocolMasterUnipump::runCommand(std::unique_ptr<UniversalAuthorize> cmd, int addr) {
    std::vector<uint8_t> crccmd;
    std::vector<uint8_t> packet;
    auto volumeInMl = cmd->getVolume();
    auto priceInCent = cmd->getPrice();
    auto nozzle = cmd->getNozzle();

    packet.push_back(DLE);
    packet.push_back(STX);
    crccmd.push_back(addr + 0x30);
    packet.push_back(addr + 0x30);
    crccmd.push_back(0x41);
    packet.push_back(0x41); /// пуск выдачи топлива
    crccmd.push_back(nozzle + 0x30);
    packet.push_back(nozzle + 0x30);
    crccmd.push_back('L');
    packet.push_back('L'); /// Ставим отпуск по обьему

    volumeInMl /= 10;
    priceInCent *= 100;
    for(int i = 0; i < 6; i++) {
        int digitVolume = static_cast<int>(volumeInMl / pow(10, 5 - i));

        crccmd.push_back(digitVolume + 0x30);
        packet.push_back(digitVolume + 0x30);
        volumeInMl -= static_cast<int>(digitVolume * pow(10, 5 - i));
    }
    for(int i = 0; i < 6; i++) {
        int digitVolume = static_cast<int>(priceInCent / pow(10, 5 - i));

        crccmd.push_back(digitVolume + 0x30);
        packet.push_back(digitVolume + 0x30);
        priceInCent -= static_cast<int>(digitVolume * pow(10, 5 - i));
    }
    auto rescrc = calc_crc(crccmd);
    uint8_t leftPart = rescrc;
    uint8_t rightPart = rescrc >> 8;
    packet.push_back(leftPart);
    packet.push_back(rightPart);

    packet.push_back(DLE);
    packet.push_back(ETX);
    return packet;
}

bool ProtocolMasterUnipump::checkPacketAppropriation(const std::vector<unsigned char> &cmd) {
    if(cmd.at(0) == DLE and checkCheckSum(cmd)) {
        return true;
    }
    return false;
}

bool ProtocolMasterUnipump::checkCheckSum(const std::vector<unsigned char>& cmd) {
    std::vector<unsigned char> checkVector;
    auto sourcePacketLength = cmd.size();

    for(int i = 2; i < sourcePacketLength - 4; i++) {
        checkVector.push_back(cmd.at(i));
    }
    auto rescrc = calc_crc(checkVector);
    unsigned char left = rescrc;
    unsigned char right = rescrc >> 8;

    if(right == cmd.at(sourcePacketLength - 3) and left == cmd.at(sourcePacketLength - 4)) {
        return true;
    } else {
        return false;
    }
}

std::shared_ptr<UniversalState>
ProtocolMasterUnipump::parseResponse(const std::vector<unsigned char> &cmd, int expectedPumpAddr, int pumpId) {
    std::shared_ptr<UniversalState> resSample;
    int activeNozzle = 0, address, amountInCent = 0x00, volumeInMl = 0x00, priceInCents = 0x00;
    int numberOfActivePistol;
    int dispensedPrice, txnId = 0;
    StateType status;
    std::vector<uint8_t> emptySourcePacket = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if(expectedPumpAddr == (cmd.at(2) - 0x30)) {
        address = pumpId;
    } else {
        address = 0;
    }
    switch(cmd.at(cmdIndex)) {
        case 0x53:   /// response on pump status
            activeNozzle = cmd.at(4) - 0x30;
            switch (cmd.at(5) - 0x30) {
                case 0:
                    status = StateType::NOT_SERVED;
                    break;
                case 1:
                    status = StateType::HANGED_FREE;
                    break;
                case 3:
                    status = StateType::REMOVED_WAITING;
                    break;
                case 4:
                    status = StateType::WAITING_APPROPRIATE_PISTOL;
                    break;
                case 8:
                    status = StateType::ERROR;
                    break;
                default:
                    status = StateType::NOT_SERVED;
                    break;
            }
            return std::make_shared<UniversalState>(UniversalState(address, activeNozzle, status, 0));
        case 0x55:   /// Unlock status response
            status = StateType::LOCK_STATUS;
            resSample = std::make_shared<UniversalState>(UniversalState(address, activeNozzle, status, 0));
            break;
        case 0x41: {   /// Response on current dispensing
            if(cmd.size() < 22) {
                return {};
            }
            activeNozzle = cmd.at(6) - 0x30;
            txnId += (cmd.at(4) - 0x30) * 10;
            txnId += (cmd.at(5) - 0x30);
            for (int i = 0; i < 6; i++) {
                amountInCent += static_cast<int>((cmd.at(i + 7) - 0x30) * pow(10, 5 - i));
            }
            for (int i = 0; i < 6; i++) {
                volumeInMl += static_cast<int>((cmd.at(i + 13) - 0x30) * pow(10, 6 - i));
            }
            if((volumeInMl / 100) > 0) {
            }
            auto resAmount = std::make_shared<UniversalAmount>(UniversalAmount(address, activeNozzle, 0, volumeInMl,
                                                               priceInCents, txnId, StateType::FUEL_SUPPLY, amountInCent));
            return resAmount;
        }
        case 0x54: {  /// END_OF_TRANSACTION
            status = StateType::SUPPLY_DONE;
            activeNozzle = cmd.at(6) - 0x30;
            txnId += (cmd.at(4) - 0x30) * 10;
            txnId += cmd.at(5) - 0x30;
            transactionHolderAddrId[cmd.at(2) - 0x30] = {cmd.at(4), cmd.at(5)};
            for (int i = 0; i < 6; i++) {
                amountInCent += static_cast<int>((cmd.at(i + 7) - 0x30) * pow(10, 5 - i));
            }
            for (int i = 0; i < 6; i++) {
                if ((cmd.at(i + 13) - 0x30) != 0) {
                    volumeInMl += static_cast<int>((cmd.at(i + 13) - 0x30) * pow(10, 6 - i));
                }
            }
            for (int i = 0; i < 4; i++) {
                if ((cmd.at(i + 19) - 0x30) != 0) {
                    priceInCents += static_cast<int>((cmd.at(i + 19) - 0x30) * pow(10, 3 - i));
                }
            }
            auto resAmount = std::make_shared<UniversalAmount>(UniversalAmount(address, activeNozzle, 0, volumeInMl,
                                                                               priceInCents, txnId, status, amountInCent));
            return resAmount;
        }
        case 0x43: {  /// response on pump nozzle total counters
            txnId = 0x00;
            numberOfActivePistol = cmd.at(6) - 0x30;
            dispensedPrice = 0x00;
            volumeInMl = 0x00;

            txnId |= cmd.at(4);
            txnId |= cmd.at(5) << 8;
            for (int i = 0; i < 8; i++) {
                dispensedPrice |= cmd.at(i + 19) << (i * 8);
            }
            for (int i = 0; i < 8; i++) {
                volumeInMl |= cmd.at(i + 7) << (i * 8);
            }
            /// int address, int nozzle, StateType _type, int txnId, int txnType, int _volume
            auto resAmount = std::make_shared<UniversalTotals>(UniversalTotals(volumeInMl, dispensedPrice));
            resAmount->setId(pumpId);
            resAmount->setNozzle(numberOfActivePistol);
            resAmount->setType(StateType::TOTAL_RESPONSE);
            return resAmount;
        }
        default:
            resSample = std::make_shared<UniversalState>(1, 0, StateType::ERROR, 0);
            break;
    }
    return resSample;
}

std::vector<unsigned char> ProtocolMasterUnipump::closeCommand(std::unique_ptr<UniversalClose> cmd, int addr) {
    std::vector<unsigned char> packet;
    std::vector<unsigned char> crccmd;
    auto pumpTransactionId = transactionHolderAddrId.at(addr);

    packet.push_back(DLE);
    packet.push_back(STX);
    packet.push_back(addr + 0x30);
    packet.push_back(0x43);    /// в документации Unipump - HaltRequest
    crccmd.push_back(addr + 0x30);
    crccmd.push_back(0x43);
    packet.push_back(pumpTransactionId.first);
    packet.push_back(pumpTransactionId.second);
    crccmd.push_back(pumpTransactionId.first);
    crccmd.push_back(pumpTransactionId.second);
    auto rescrc = calc_crc(crccmd);
    uint8_t leftPart = rescrc;
    uint8_t rightPart = rescrc >> 8;
    packet.push_back(leftPart);
    packet.push_back(rightPart);
    packet.push_back(DLE);
    packet.push_back(ETX);
    return packet;
}

std::vector<unsigned char> ProtocolMasterUnipump::stopCommand(std::unique_ptr<UniversalCmd> cmd, int addr) {
    std::vector<unsigned char> packet;
    std::vector<unsigned char> crccmd;

    packet.push_back(DLE);
    packet.push_back(STX);
    packet.push_back(addr + 0x30);
    packet.push_back(0x48);    /// в документации Unipump - HaltRequest
    crccmd.push_back(addr + 0x30);
    crccmd.push_back(0x48);
    auto rescrc = calc_crc(crccmd);
    uint8_t leftPart = rescrc;
    uint8_t rightPart = rescrc >> 8;
    packet.push_back(leftPart);
    packet.push_back(rightPart);
    packet.push_back(DLE);
    packet.push_back(ETX);
    return packet;
}


std::vector<unsigned char> ProtocolMasterUnipump::setPriceRequest(std::unique_ptr<UniversalCmd> cmd, int addr) {
    auto authorizeCmd = std::make_unique<UniversalAuthorize>(*dynamic_cast<UniversalAuthorize*>(cmd.release()));
    int priceIn = authorizeCmd->getPrice();
    int workingPitsol = 1;
    std::vector<unsigned char> packet;
    std::vector<unsigned char> crccmd;

    packet.push_back(DLE);
    packet.push_back(STX);
    packet.push_back(addr + 0x30);
    packet.push_back(0x70); /// Команда для виставления цен
    crccmd.push_back(addr + 0x30);
    crccmd.push_back(0x70);
    for(int i = 4; i < 28; i += 4) {
         for(int j = 0; j < 4; j++) {
            int digitVolume = static_cast<int>(priceIn / pow(10, 3 - j));
            crccmd.push_back(digitVolume + 0x30);
            packet.push_back(digitVolume + 0x30);
            priceIn -= static_cast<int>(digitVolume * pow(10, 3 - j));
        }
    }
    auto rescrc = calc_crc(crccmd);
    uint8_t leftPart = rescrc;
    uint8_t rightPart = rescrc >> 8;
    packet.push_back(leftPart);
    packet.push_back(rightPart);
    packet.push_back(DLE);
    packet.push_back(ETX);
    return packet;
}

unsigned int ProtocolMasterUnipump::calc_crc(const std::vector<unsigned char> &cmd) {
    return getCrc16(0, cmd);
}