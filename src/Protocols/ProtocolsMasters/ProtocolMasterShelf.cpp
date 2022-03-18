#include "ProtocolMasterShelf.h"

const int maxNozzles = 7;

int ProtocolMasterShelf::calcrc(std::vector<unsigned char> ptr, int count) {
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

/*
 *  D0 – бит указывающий что как минимум один пистолет снят D1 – бит снятия первого пистолета ( активная “1”)
    D2 – бит снятия второго пистолета ( активная “1”)
    D3 – бит снятия третьего пистолета ( активная “1”)
    D4 – бит снятия четвёртого пистолета ( активная “1”)
    D5 – бит снятия пятого пистолета ( активная “1”)
    D6,D7 – зарезервированы для дальнейших применений
 */

int ProtocolMasterShelf::resolveActiveNozzle(unsigned char nozzleStateByte) {
    int activeNozzle = 0;

    for(int nozzleIndex = 1; nozzleIndex < maxNozzles; nozzleIndex++) {
        if((nozzleStateByte >> nozzleIndex) & 1) {
            activeNozzle = nozzleIndex;
            break;
        }
    }
    return activeNozzle;
}

/*
 *  D0 – на данной стороне ТРК снят хоть один пистолет
    D1 – отсечной клапан включён
    D2 – замедляющий клапан активен
    D3 – для данной стороны двигатель данного вида топлива в работе D4 – бит приостанова ТРК
    D5 – бит останова ТРК
    D6 – бит активности клавиатуры
    D7 – бит налива ТРК по данному топливу по данной стороне
 */

StateType ProtocolMasterShelf::resolveTrkState(unsigned char trkStateByte) {
    int activeByte = 0;

    for(int trkIndex = 0; trkIndex < maxNozzles; trkIndex++) {
        if((trkStateByte >> trkIndex) & 1) {
            activeByte = trkIndex;
            break;
        }
    }
    switch(activeByte) {
        case 0:
        case 6:
            return StateType::REMOVED_WAITING;
        case 5:
            return StateType::HANGED_FREE;
        case 1:
        case 2:
        case 3:
        case 4:
        case 7:
            return StateType::FUEL_SUPPLY;
        default:
            return StateType::NOT_SERVED;
    }
}

[[maybe_unused]] std::vector<unsigned char> ProtocolMasterShelf::runCommand(std::unique_ptr<UniversalAuthorize> cmd, int addr) {
    std::vector<unsigned char> comandReq;
    int volumeInShelfUnits = cmd->getVolume() / 10; /// Need to convert from 1000 ml to 100 for Shelf
    int priceInCents = cmd->getPrice();

    currentPrices[addr] = priceInCents;
    comandReq.push_back(0x2D);
    comandReq.push_back(addr);
    comandReq.push_back(getCounterRequest(addr));
    comandReq.push_back(0x0E);
    comandReq.push_back(0x05); /// (WriteVolume)( WV )
    comandReq.push_back(0x00); /// Скидка 1
    comandReq.push_back(0x00); /// Скидка 2
    comandReq.push_back(volumeInShelfUnits);
    comandReq.push_back(volumeInShelfUnits >> 8);
    comandReq.push_back(volumeInShelfUnits >> 16);
    comandReq.push_back(priceInCents);
    comandReq.push_back(priceInCents >> 8);
    uint16_t crc = calcrc(comandReq, 12);
    comandReq.push_back(crc);
    comandReq.push_back(crc >> 8);
    return comandReq;
}

unsigned int ProtocolMasterShelf::getCounterRequest(unsigned char _address) {
    if(packetIndexCounter.find(_address) != packetIndexCounter.end()) {
        auto indexOnAddress = packetIndexCounter.at(_address);

        if (indexOnAddress == 0xFF) {
            packetIndexCounter[_address] = 0x00;
            return 0x00;
        } else {
            packetIndexCounter[_address]++;
            return indexOnAddress + 1;
        }
    } else {
        packetIndexCounter[_address] = 0x00;
        return 0x00;
    }
}

[[maybe_unused]] std::vector<unsigned char> ProtocolMasterShelf::statusRequest(std::unique_ptr<UniversalCmd> cmd, int addr) {
    std::vector<unsigned char> comandReq;

    comandReq.push_back(0x2D);
    comandReq.push_back(addr);
    comandReq.push_back(getCounterRequest(addr));
    comandReq.push_back(0x07);
    comandReq.push_back(0x01); /// ( Status Request )(SR)
    uint16_t crc = calcrc(comandReq, 5);
    comandReq.push_back(crc);
    comandReq.push_back(crc >> 8);
    return comandReq;
}

[[maybe_unused]] std::vector<unsigned char> ProtocolMasterShelf::stopCommand(std::unique_ptr<UniversalCmd> cmd, int addr) {
    std::vector<unsigned char> comandReq;

    comandReq.push_back(0x2D);
    comandReq.push_back(addr);
    comandReq.push_back(getCounterRequest(addr));
    comandReq.push_back(0x07);
    comandReq.push_back(0x0C); /// Stop Command (SC)
    uint16_t crc = calcrc(comandReq, 5);
    comandReq.push_back(crc);
    comandReq.push_back(crc >> 8);
    return comandReq;
}

std::vector<unsigned char> ProtocolMasterShelf::totalCommand(std::unique_ptr<UniversalTotal> cmd, int addr) {
    std::vector<unsigned char> comandReq;

    comandReq.push_back(0x2D);
    comandReq.push_back(addr);
    comandReq.push_back(getCounterRequest(addr));
    comandReq.push_back(0x07);
    comandReq.push_back(0x15); /// Total Counters Request (TCR)
    uint16_t crc = calcrc(comandReq, 5);
    comandReq.push_back(crc);
    comandReq.push_back(crc >> 8);
    return comandReq;
}

[[maybe_unused]] std::vector<unsigned char> ProtocolMasterShelf::setPriceRequest(std::unique_ptr<UniversalAuthorize> cmd, int addr) {
    std::vector<unsigned char> comandReq;
    int priceInShelfUnit = cmd->getPrice(); /// в качестве данных указывается цена за 1 м3 топлива в копейках. Максимальная цена 9999 коп.

    comandReq.push_back(0x2D);
    comandReq.push_back(addr);
    comandReq.push_back(getCounterRequest(addr));
    comandReq.push_back(0x09);
    comandReq.push_back(0x03); /// Write Price (WP)
    comandReq.push_back(priceInShelfUnit);
    comandReq.push_back(priceInShelfUnit >> 8);
    uint16_t crc = calcrc(comandReq, 7);
    comandReq.push_back(crc);
    comandReq.push_back(crc >> 8);
    return comandReq;
}

std::vector<unsigned char> ProtocolMasterShelf::traceResponse(std::vector<unsigned char> packet) {
    return packet;
}

std::vector<unsigned char> ProtocolMasterShelf::traceRequests(std::vector<unsigned char> packet) {
    return packet;
}

std::vector<unsigned char> ProtocolMasterShelf::preparePacket(std::unique_ptr<UniversalCmd> cmd, int addr) {
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
            packet = stopCommand(std::move(std::make_unique<UniversalClose>(*dynamic_cast<UniversalClose*>(cmd.release()))), addr);
            break;
        default:
            LogPrintf(spdlog::level::warn, "Cannot form packet, undefined command. Code: {}", cmd->getCmdType());
            break;
    }
    return packet;
}

std::shared_ptr<UniversalState> ProtocolMasterShelf::resolveStopStatus(const std::vector<unsigned char> &cmd) {
    int addr = cmd.at(1);
    auto nozzleStatus = cmd.at(5);
    auto trkStatus = cmd.at(6);
    int activeNozzle = resolveActiveNozzle(nozzleStatus);
    auto trkState = resolveTrkState(trkStatus);

    return std::make_shared<UniversalState>(addr, activeNozzle, trkState, 0);
}

std::shared_ptr<UniversalAmount> ProtocolMasterShelf::resolveAmountInfo(const std::vector<unsigned char>& cmd) {
    int addr = cmd.at(1);
    int volume = 0, sum = 0, price = 0;
    int activeNozzle = resolveActiveNozzle(cmd.at(5));
    auto trkState = resolveTrkState(cmd.at(6));

    if(currentPrices.find(addr) != currentPrices.end()) {
        price = currentPrices.at(addr);
    } else if(currentPrices.find(addr + activeNozzle) != currentPrices.end()) {
        price = currentPrices.at(addr + activeNozzle);
    }

    volume |= cmd.at(9);
    volume |= cmd.at(10) << 8;
    volume |= cmd.at(11) << 16;
    volume *= 10; /// We use ML
    sum = (volume * price) / 1000;

    return std::make_shared<UniversalAmount>(addr, activeNozzle, 0, volume, price, 0, trkState, sum);
}

std::shared_ptr<UniversalAmount> ProtocolMasterShelf::resolveMadeAmount(const std::vector<unsigned char>& cmd) {
    int addr = cmd.at(1);
    int volume = 0, sum = 0, price = 0;
    int activeNozzle = resolveActiveNozzle(cmd.at(5));
    auto trkState = resolveTrkState(cmd.at(6));

    if(currentPrices.find(addr) != currentPrices.end()) {
        price = currentPrices.at(addr);
    } else if(currentPrices.find(addr + activeNozzle) != currentPrices.end()) {
        price = currentPrices.at(addr + activeNozzle);
    }

    volume |= cmd.at(9);
    volume |= cmd.at(10) << 8;
    volume |= cmd.at(11) << 16;
    volume *= 10; /// We use ML
    sum = (volume * price) / 1000;

    return std::make_shared<UniversalAmount>(addr, activeNozzle, 0, volume, price, 0, StateType::SUPPLY_DONE, sum);
}

std::shared_ptr<UniversalTotals> ProtocolMasterShelf::resolveTotal(const std::vector<unsigned char>& cmd) {
    long long totalVolume = 0;
    int addr = cmd.at(1);
    int activeNozzle = resolveActiveNozzle(cmd.at(5));
    auto trkState = resolveTrkState(cmd.at(6));

    totalVolume |= cmd.at(5);
    totalVolume |= cmd.at(6) << 8;
    totalVolume |= cmd.at(7) << 16;
    totalVolume |= cmd.at(8) << 24;
    totalVolume *= 10; /// We use ML
    return std::make_shared<UniversalTotals>(addr, totalVolume, 0, activeNozzle);
}

std::shared_ptr<UniversalAuthorizeConfirmation> ProtocolMasterShelf::resolveActiveKeyboard (const std::vector<unsigned char>& cmd, bool ready) {
    int addr =  cmd.at(1);
    int requestedVolume = 0;

    requestedVolume |= cmd.at(7);
    requestedVolume |= cmd.at(8) << 8;
    requestedVolume |= cmd.at(9) << 16;

    if(ready) {
        return std::make_shared<UniversalAuthorizeConfirmation>(addr, requestedVolume, StateType::AUTHORIZE_REGISTERED);
    } else {
        return std::make_shared<UniversalAuthorizeConfirmation>(addr, requestedVolume, StateType::REMOVED_MANUAL_DOSING);
    }
}

std::shared_ptr<UniversalAmount> ProtocolMasterShelf::resolveCurrentAmount(const std::vector<unsigned char>& cmd) {
    int addr = cmd.at(1);
    int volume = 0, sum = 0, price = 0;
    int activeNozzle = resolveActiveNozzle(cmd.at(5));
    auto trkState = resolveTrkState(cmd.at(6));

    if(currentPrices.find(addr) != currentPrices.end()) {
        price = currentPrices.at(addr);
    } else if(currentPrices.find(addr + activeNozzle) != currentPrices.end()) {
        price = currentPrices.at(addr + activeNozzle);
    }

    volume |= cmd.at(9);
    volume |= cmd.at(10) << 8;
    volume |= cmd.at(11) << 16;
    volume *= 10; /// We use ML
    sum = (volume * price) / 1000;

    return std::make_shared<UniversalAmount>(addr, activeNozzle, 0, volume, price, 0, trkState, sum);
}

std::shared_ptr<UniversalState>
ProtocolMasterShelf::parseResponse(const std::vector<unsigned char> &cmd, int expectedPumpAddr, int pumpId) {
    std::shared_ptr<UniversalState> resSample;
    auto command = cmd.at(commandByte);

    switch(command) {
        case 0x81: // Stop Status Response
            resSample = resolveStopStatus(cmd);
            break;
        case 0x82: // Active keyboard response
            resSample = resolveActiveKeyboard(cmd, false);
            break;
        case 0x83: // Active keyboard response
            resSample = resolveActiveKeyboard(cmd, true);
            break;
        case 0x84: // Active Mode Response (AMR) - код ответа о успешном выполнении принятой команды и переходе ТРК в состояние налива ( или нахождении ТРК в состоянии налива )
            resSample = resolveAmountInfo(cmd);
            break;
        case 0xA0: // Total Counters Request (TCR)
            resSample = resolveTotal(cmd);
            break;
        case 0x91: // Read Price Response (RPR)
        case 0x94: // Read Volume of Change (RVC) - ответ при успешном чтении литров за текущую смену.
        case 0x95: // Read Volume of Change non-Cash (RVCC) - ответ при успешном чтении безналичного объёма за текущую смену
        case 0x96: // Cash Money of Change Response (CMCR) - ответ при успешном чтении суммы наличных денег за текущую смену
        case 0x97: // Passage Volume of Change Response (PVCR) - ответ при успешном чтении объёма при операции “прогон” за текущую смену
        case 0x00: // Command Carry out Response (CCR) – код ответа об успешном выполнении принятой команды .
        case 0xFF: // Command Bad Response (CBR) - код ответа о невозможности выполнить принятую команду
        case 0x85: // Another Gun Current Amount Response (AGCAR) - ТРК выдаёт подобный ответ в случае , если ВУУ запрашивает ТРК , находящееся в состоянии отпуска топлива, по адресу , не совпадающему с активным адресом в данном наливе.
            LogFile(spdlog::level::warn, "|SHELF| Undone command from |TRK|: |{}|", command);
            resSample->setType(StateType::ERROR);
            break;
        case 0x92: // Current Amount Response (CAR) – ответ о текущем отпущенном объёме
            resSample = resolveCurrentAmount(cmd);
        case 0x93: // Made Amount Response (MAR)
            resSample = resolveMadeAmount(cmd);
            break;
        default:
            LogPrintf(spdlog::level::warn, "|SHELF| Unknown command from |TRK|: |{}|", command);
    }
    resSample->setId(pumpId);
    return resSample;
}

bool ProtocolMasterShelf::checkPacketAppropriation(const std::vector<unsigned char> &cmd) {
    /// TODO: review minimum packet size
    if(cmd.size() < 6) {
        return false;
    }
    return true;
}

