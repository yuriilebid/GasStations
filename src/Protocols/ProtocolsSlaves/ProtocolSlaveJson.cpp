#include "ProtocolSlaveJson.h"
#include <UniversalAuthorize.h>
#include <UniversalClose.h>
#include "logger.h"
#include <TechCmd.h>
#include "UniversalTech.h"
#include "UniversalPrice.h"
#include "UniversalTotal.h"

float roundoff(float value, unsigned char prec) {
    float pow_10 = pow(10.0f, (float)prec);
    return round(value * pow_10) / pow_10;
}

std::string ProtocolSlaveJson::gatherResponses(const std::vector<std::string> &responses) {
    nlohmann::json responseObj;

    responseObj["Protocol"] = "jsonPTS";
    for(auto& singleResponse : responses) {
        nlohmann::json singlePacket = nlohmann::json::parse(singleResponse);

        responseObj["Packets"].push_back(singlePacket);
        }
    return responseObj.dump();
}

std::vector<std::vector<unsigned char>> ProtocolSlaveJson::splitPackets(const std::vector<unsigned char>& srcPacket) {
    std::vector<std::vector<unsigned char>> resPackets;
    auto packetObj = nlohmann::json::parse(srcPacket);

    try {
        for (auto &singlePacket: packetObj.at("Packets")) {
            const std::string packet = singlePacket.dump();
            resPackets.emplace_back(packet.begin(), packet.end());
        }
    } catch(nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "Incorrect Json Packet: {}", e.what());
    }
    return resPackets;
}

std::unique_ptr<UniversalCmd> ProtocolSlaveJson::parseCmd(const std::vector<unsigned char>& cmd) {
    auto concretePacket = nlohmann::json::parse(cmd);

    try {
        packetId = concretePacket.at("Id");
        switch(concretePacket.at("Type").get<jsonCmdType>()) {
            case jsonCmdType::STATUS: {
                int pumpId = concretePacket.at("Data").at("Pump");

                std::unique_ptr<UniversalCmd> resCmd = std::make_unique<UniversalCmd>(pumpId, CmdType::STATUS_REQUEST);
                return resCmd;
            }
            case jsonCmdType::AUTHORIZE: {
                auto singleCmd = std::make_unique<UniversalAuthorize>();
                singleCmd->setId(concretePacket.at("Data").at("Pump").get<int>());
                singleCmd->setCmdType(CmdType::WRITE_VOLUME);
                singleCmd->setNozzle(concretePacket.at("Data").at("Nozzle").get<int>());
                if(concretePacket.at("Data").contains("Price")) {
                    if(concretePacket.at("Data").at("Price").type() == nlohmann::json::value_t::number_float or
                    concretePacket.at("Data").at("Price").type() == nlohmann::json::value_t::number_integer) {
                        singleCmd->setPrice(static_cast<int>((concretePacket.at("Data").at("Price").get<float>() + 0.005) * 100.0));
                    } else {
                        singleCmd->setPrice(
                                static_cast<int>((std::stof(concretePacket.at("Data").at("Price").get<std::string>()) + 0.005) *
                                                 100.0));
                    }
                } else {
                    LogPrintf(spdlog::level::warn, "|JSON| Setting default price: 1000");
                    singleCmd->setPrice(1000);
                }
                if(concretePacket.at("Data").at("Type") == "FullTank") {
                    singleCmd->setVolume(999000);
                } else {
                    if(concretePacket.at("Data").at("Dose").type() == nlohmann::json::value_t::number_float or
                        concretePacket.at("Data").at("Dose").type() == nlohmann::json::value_t::number_integer or
                        concretePacket.at("Data").at("Dose").type() == nlohmann::json::value_t::number_unsigned) {
                        LogFile(spdlog::level::info, "|JSON| Data->Dose float");
                        LogPrintf(spdlog::level::info, "|JSON| Data->Dose float");
                        singleCmd->setVolume(static_cast<int>(concretePacket.at("Data").at("Dose").get<float>() * 1000.0));
                    } else if(concretePacket.at("Data").at("Dose").type() == nlohmann::json::value_t::string) {
                        LogFile(spdlog::level::info, "|JSON| Data->Dose string");
                        LogPrintf(spdlog::level::info, "|JSON| Data->Dose string");
                        singleCmd->setVolume(
                                static_cast<int>(std::stof(concretePacket.at("Data").at("Dose").get<std::string>()) *
                                                 1000.0));
                    } else {
                        LogPrintf(spdlog::level::info, "|JSON| Data->Dose undefined type: {}", concretePacket.at("Data").at("Dose").type());
                        LogFile(spdlog::level::info, "|JSON| Data->Dose undefined type: {}", concretePacket.at("Data").at("Dose").type());
                        singleCmd->setVolume(static_cast<int>(concretePacket.at("Data").at("Dose").get<float>() * 1000.0));
                    }
                }
                return singleCmd;
            }
            case jsonCmdType::TOTALS: {
                auto singleCmd = std::make_unique<UniversalTotal>(concretePacket.at("Data").at("Nozzle").get<int>());
                singleCmd->setId(concretePacket.at("Data").at("Pump").get<int>());
                singleCmd->setCmdType(CmdType::GET_TOTAL);
                return singleCmd;
            }
            case jsonCmdType::CLOSE: {
                auto singleCmd = std::make_unique<UniversalClose>();
                singleCmd->setId(concretePacket.at("Data").at("Pump"));
                singleCmd->setTransactionId(concretePacket.at("Data").at("Transaction"));
                singleCmd->setCmdType(CmdType::CLOSE_REPORT);
                return singleCmd;
            }
            case jsonCmdType::STOP: {
                auto singleCmd = std::make_unique<UniversalCmd>();
                singleCmd->setId(concretePacket.at("Data").at("Pump"));
                singleCmd->setCmdType(CmdType::STOP_COMMAND);
                return singleCmd;
            }
            case jsonCmdType::SIMULATE: {
                if(concretePacket.at("Data").contains("State")) {
                    if(concretePacket.at("Data").at("State") == "Off") {
                        return std::make_unique<UniversalCmd>(concretePacket.at("Data").at("Pump"), CmdType::CONFIRMATION);
                    } else {
                        LogPrintf(spdlog::level::info, "|TECH_CMD| Changing scenario");
                    auto singleCmd = std::make_unique<TechCmd>(concretePacket.at("Data").at("Pump"),
                                                               concretePacket.at("Data").at("NozzleUp"),
                                                               LogicType::SLEEP);
                    return singleCmd;
                    }
                } else {
                    LogPrintf(spdlog::level::info, "|TECH_CMD| Changing scenario");
                    auto singleCmd = std::make_unique<TechCmd>(concretePacket.at("Data").at("Pump"),
                                                               concretePacket.at("Data").at("NozzleUp"),
                                                               LogicType::SLEEP);
                    return singleCmd;
                }
            }
            case jsonCmdType::SD_INFO: {
                auto singleCmd = std::make_unique<TechCmd>();
                singleCmd->setCmdType(CmdType::GET_SD);
                return singleCmd;
            }
            case jsonCmdType::BATTERY: {
                auto singleCmd = std::make_unique<TechCmd>();
                singleCmd->setCmdType(CmdType::GET_BATTERY);
                return singleCmd;
            }
            case jsonCmdType::FUEL_CONFIG: {
                auto singleCmd = std::make_unique<TechCmd>();
                singleCmd->setCmdType(CmdType::GRADES_CONFIG);
                return singleCmd;
            }
            case jsonCmdType::PUMP_CONFIG: {
                auto singleCmd = std::make_unique<TechCmd>();
                singleCmd->setCmdType(CmdType::PUMP_CONFIG);
                return singleCmd;
            }
            case jsonCmdType::SET_PRICE: {
                auto singleCmd = std::make_unique<UniversalPrice>();
                for(auto& unitPrice : concretePacket.at("Data").at("FuelGrades")) {
                    singleCmd->addProduct(unitPrice.at("Id"), unitPrice.at("Name"), unitPrice.at("Price"));
                }
                singleCmd->setCmdType(CmdType::WRITE_PRICE);
                return singleCmd;
            }
            case jsonCmdType::SET_DATE: {
                auto singleCmd = std::make_unique<TechCmd>();
                singleCmd->setCmdType(CmdType::GET_SD);
                return singleCmd;
            }
            default:
                LogPrintf(spdlog::level::warn, "Not appropriate command type");
                auto singleCmd = std::make_unique<TechCmd>();
                singleCmd->setCmdType(CmdType::GET_SD);
                return singleCmd;
        }
    } catch(nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "|JSON| Error: {}", e.what());
        return std::make_unique<UniversalCmd>(1, CmdType::STATUS_REQUEST);
    } catch(...) {
        LogPrintf(spdlog::level::err, "|JSON| Error");
        std::exception_ptr p = std::current_exception();
        return std::make_unique<UniversalCmd>(1, CmdType::STATUS_REQUEST);
    }
    return std::make_unique<UniversalCmd>(1, CmdType::STATUS_REQUEST);
}

std::string ProtocolSlaveJson::resolveHangedFreeResponse(const std::shared_ptr<UniversalState>& cmd) {
    nlohmann::json responseObj;
    nlohmann::json dataObj;
    int pumpId = cmd->getId();

    dataObj["Pump"] = pumpId;
    dataObj["NozzleUp"] = 0;
    if(lastDispense.count(pumpId)) {
        /// static_cast<float>(cmd->getVolume()) / 1000.0;
        dataObj["LastNozzle"] = lastDispense.at(pumpId).getNozzle();
        dataObj["LastVolume"] = std::ceil(static_cast<float>(lastDispense.at(pumpId).getVolume()) / 1000.0 * 100.0) / 100.0;
        dataObj["LastPrice"] = std::ceil(static_cast<float>(lastDispense.at(pumpId).getPrice())) / 100.0;
        dataObj["LastAmount"] = std::ceil(static_cast<float>(lastDispense.at(pumpId).getSum())) / 100.0;
        dataObj["LastTransaction"] = lastDispense.at(pumpId).getDbTransactionId();
    } else {
        dataObj["LastNozzle"] = 0;
        dataObj["LastVolume"] = 0;
        dataObj["LastPrice"] = 0;
        dataObj["LastAmount"] = 0;
        dataObj["LastTransaction"] = 0;
    }
    dataObj["User"] = cmd->getControlUser();
    dataObj["Request"] = "PumpGetStatus";
    responseObj["Id"] = packetId;
    responseObj["Data"] = dataObj;
    responseObj["Type"] = "PumpIdleStatus";
    return responseObj.dump();
}

/**
   "Id":1,
   "Type":"PumpIdleStatus",
   "Data":{
     "Pump":1,
     "NozzleUp":2,
     "LastNozzle":2,
     "LastVolume":5.00,
     "LastPrice":2.50,
     "LastAmount":12.50,
     "LastTransaction":3,
     "Request":"PumpAuthorize",
     "User":"admin"
}
 */
std::string ProtocolSlaveJson::resolveRemovedResponse(const std::shared_ptr<UniversalState>& cmd) {
    nlohmann::json responseObj;
    nlohmann::json dataObj;

    int pumpId = cmd->getId();
    dataObj["Pump"] = pumpId;
    dataObj["NozzleUp"] = cmd->getNozzle();
    if(lastDispense.count(pumpId)) {
        dataObj["LastNozzle"] = lastDispense.at(pumpId).getNozzle();
        dataObj["LastVolume"] = std::ceil(static_cast<float>(lastDispense.at(pumpId).getVolume()) / 1000.0 * 100.0) / 100.0;
        dataObj["LastPrice"] = std::ceil(static_cast<float>(lastDispense.at(pumpId).getPrice())) / 100.0;
        dataObj["LastAmount"] = std::ceil(static_cast<float>(lastDispense.at(pumpId).getSum())) / 100.0;
        dataObj["LastTransaction"] = lastDispense.at(pumpId).getDbTransactionId();
    } else {
        dataObj["LastNozzle"] = 0;
        dataObj["LastVolume"] = 0;
        dataObj["LastPrice"] = 0;
        dataObj["LastAmount"] = 0;
        dataObj["LastTransaction"] = 0;
    }
    dataObj["User"] = cmd->getControlUser();
    dataObj["Request"] = "PumpGetStatus";
    responseObj["Id"] = packetId;
    responseObj["Data"] = dataObj;
    responseObj["Type"] = "PumpIdleStatus";
    return responseObj.dump();
}

std::vector<unsigned char> ProtocolSlaveJson::traceResponse(std::vector<unsigned char> packet) {
    auto responseJson = nlohmann::json::parse(packet);

    LogFile(spdlog::level::trace, "|JSON||READ|: {}", packet.data());
    LogConsole(spdlog::level::trace, "|JSON||READ|: {}", packet.data());
    return packet;
}

std::vector<unsigned char> ProtocolSlaveJson::traceRequests(std::vector<unsigned char> packet) {
    auto responseJson = nlohmann::json::parse(packet);

    for(auto& packetObj : responseJson["Packets"]) {
        if(packetObj.contains("Type")) {
            if(packetObj["Type"].get<std::string>() == "BatteryVoltage" or
            packetObj["Type"].get<std::string>() == "SdInformation") {
                return {};
            } else if(packetObj["Type"].get<std::string>() == "PumpIdleStatus") {
                if(lastResponseStatus != packet) {
                    lastResponseStatus = packet;
                    const std::string msg = reinterpret_cast<char*>(packet.data());
                    LogConsole(spdlog::level::trace, "|JSON||WRITE|: {}", msg);
                    LogFile(spdlog::level::trace, "|JSON||WRITE|: {}", msg);
                    return {};
                }
            } else {
                if(lastResponse != packet) {
                    lastResponse = packet;
                    lastResponseStatus = packet;
                    const std::string msg = reinterpret_cast<char*>(packet.data());
                    LogConsole(spdlog::level::trace, "|JSON||WRITE|: {}", msg);
                    LogFile(spdlog::level::trace, "|JSON||WRITE|: {}", msg);
                    return {};
                }
            }
        }
    }
    return {};
}

std::string ProtocolSlaveJson::resolveFuelSupply(const std::shared_ptr<UniversalAmount>& cmd) {
    nlohmann::json responseObj;
    nlohmann::json dataObj;

    dataObj["Pump"] = cmd->getId();
    dataObj["Nozzle"] = cmd->getNozzle();
    dataObj["Volume"] = std::ceil(static_cast<float>(cmd->getVolume()) / 1000.0 * 100.0) / 100.0;
    dataObj["Price"] = std::ceil(static_cast<float>(cmd->getPrice())) / 100.0;
    dataObj["Amount"] = std::ceil(static_cast<float>(cmd->getSum())) / 100.0;
    dataObj["Transaction"] = cmd->getDbTransactionId();
    dataObj["User"] = cmd->getControlUser();
    responseObj["Id"] = packetId;
    responseObj["Data"] = dataObj;
    responseObj["Type"] = "PumpFillingStatus";
    return responseObj.dump();
}

std::string ProtocolSlaveJson::resolveSupplyDone(const std::shared_ptr<UniversalAmount>& cmd) {
    nlohmann::json responseObj;
    nlohmann::json dataObj;
    int colId = cmd->getId();

    dataObj["Pump"] = colId;
    dataObj["Nozzle"] = cmd->getNozzle();
    dataObj["Volume"] = std::ceil(static_cast<float>(cmd->getVolume()) / 1000.0 * 100.0) / 100.0;
    dataObj["TCVolume"] = std::ceil(static_cast<float>(cmd->getVolume()) / 1000.0 * 100.0) / 100.0;
    dataObj["Price"] = std::ceil(static_cast<float>(cmd->getPrice())) / 100.0;
    dataObj["Amount"] = std::ceil(static_cast<float>(cmd->getSum())) / 100.0;
    dataObj["Transaction"] = cmd->getDbId();
    dataObj["User"] = cmd->getControlUser();
    responseObj["Id"] = packetId;
    responseObj["Data"] = dataObj;
    responseObj["Type"] = "PumpEndOfTransactionStatus";


    UniversalAmount newLastTransaction;
    newLastTransaction.setId(colId);
    newLastTransaction.setVolume(cmd->getVolume());
    newLastTransaction.setSum(cmd->getSum());
    newLastTransaction.setPrice(cmd->getPrice());
    newLastTransaction.setNozzle(cmd->getNozzle());
    newLastTransaction.setTxnId(cmd->getDbId());
    lastDispense[colId] = newLastTransaction;
    return responseObj.dump();
}

std::string ProtocolSlaveJson::resolveConfirmation() {
    nlohmann::json confirmationObj;

    confirmationObj["Id"] = packetId;
    confirmationObj["Message"] = "OK";
    return confirmationObj.dump();
}

std::string ProtocolSlaveJson::resolveError(const std::string &errMsg) {
    nlohmann::json errorObj;

    errorObj["Id"] = packetId;
    errorObj["Error"] = true;
    errorObj["Message"] = errMsg;
    return errorObj.dump();
}

std::string ProtocolSlaveJson::resolveTotals(const std::shared_ptr<UniversalTotals>& cmd) {
    nlohmann::json responseObj;
    nlohmann::json dataObj;

    dataObj["Pump"] = cmd->getId();
    dataObj["Nozzle"] = cmd->getNozzle();
    dataObj["Volume"] = std::ceil(static_cast<double>(cmd->getVolume()) / 1000.0 * 100.0) / 100.0;
    dataObj["Amount"] = std::ceil(static_cast<double>(cmd->getSum())) / 100.0;
    dataObj["Transaction"] = 1;
    dataObj["User"] = cmd->getControlUser();
    responseObj["Id"] = packetId;
    responseObj["Data"] = dataObj;
    responseObj["Type"] = "PumpTotals";
    return responseObj.dump();
}

std::vector<unsigned char> ProtocolSlaveJson::preparePacketVec(std::shared_ptr<UniversalState> cmd, int addr) {
    return {};
}

std::string ProtocolSlaveJson::resolveConfigurationResponse(std::unique_ptr<UniversalGradesConfig> cfg) {
    auto productCfg = cfg->getProductsConfiguration();
    nlohmann::json responsePacket, data, fuleGrade;

    responsePacket["Id"] = packetId;
    responsePacket["Type"] = "FuelGradesConfiguration";
    for(const auto&[productId, product]  : productCfg) {
        fuleGrade["Id"] = productId;
        fuleGrade["Name"] = product.getName();
        fuleGrade["Price"] = product.getPrice();
        data["FuelGrades"].push_back(fuleGrade);
    }
    responsePacket["Data"] = data;
    return responsePacket.dump();
}

std::string ProtocolSlaveJson::resolveLogsResponse(std::unique_ptr<UniversalTech> cmd) {
    return {};
}

std::string ProtocolSlaveJson::resolvePumpConfigurationResponse(std::unique_ptr<UniversalPumpsConfig> data) {
    auto productCfg = data->getPumpsConfig();
    nlohmann::json responsePacket, dataPacket;

    responsePacket["Id"] = packetId;
    responsePacket["Type"] = "PumpNozzlesConfiguration";
    for(const auto&[pump, product]: productCfg) {
        nlohmann::json pumpNozzles;

        pumpNozzles["PumpId"] = pump;
        for(auto& productNozzle : product) {
            pumpNozzles["FuelGradeIds"].push_back(productNozzle);
        }
        dataPacket["PumpNozzles"].push_back(pumpNozzles);
    }
    responsePacket["Data"] = dataPacket;
    return responsePacket.dump();
}

std::string ProtocolSlaveJson::resolveBatteryResponse() {
    nlohmann::json responsePacket, dataPacket, pumpNozzles;

    responsePacket["Id"] = packetId;
    responsePacket["Type"] = "BatteryVoltage";
    dataPacket["Voltage"] = 2950;
    responsePacket["Data"] = dataPacket;
    return responsePacket.dump();
}

std::string ProtocolSlaveJson::resolveSdResponse() {
    nlohmann::json responsePacket, dataPacket, pumpNozzles, file;

    responsePacket["Id"] = packetId;
    responsePacket["Type"] = "SdInformation";
    file["Name"] = "PtsLog.txt";
    file["Size"] = "12205";
    dataPacket["Files"].push_back(file);
    dataPacket["FreeMemoryKB"] = 7780544;
    dataPacket["TotalMemoryKB"] = 7781376;
    responsePacket["Data"] = dataPacket;
    return responsePacket.dump();
}

std::string ProtocolSlaveJson::resolveNotServerResponse(const std::shared_ptr<UniversalState>& cmd) {
    nlohmann::json responsePacket, dataPacket, pumpNozzles, file;

    responsePacket["Id"] = packetId;
    responsePacket["Type"] = "PumpOfflineStatus";
    dataPacket["Pump"] = cmd->getId();
    dataPacket["User"] = cmd->getControlUser();
    responsePacket["Data"] = dataPacket;
    return responsePacket.dump();
}

std::string ProtocolSlaveJson::preparePacketStr(std::unique_ptr<UniversalTech> cmd) {
    switch(cmd->getType()) {
        case techCmdType::CONFIGURATION:
            return resolveConfigurationResponse(std::move(std::make_unique<UniversalGradesConfig>(*dynamic_cast<UniversalGradesConfig*>(cmd.release()))));
        case techCmdType::LOGS:
            return resolveLogsResponse(std::move(cmd));
        case techCmdType::PUMP_CONFIGURATION:
            return resolvePumpConfigurationResponse(std::move(std::make_unique<UniversalPumpsConfig>(*dynamic_cast<UniversalPumpsConfig*>(cmd.release()))));
        case techCmdType::BATTERY:
            return resolveBatteryResponse();
        case techCmdType::SD_INFO:
            return resolveSdResponse();
        case techCmdType::SET_PRICE:
        case techCmdType::SCENARIO:
            return resolveConfirmation();
        default:
            break;
    }
    return {};
}

std::string ProtocolSlaveJson::resolveAuthorizeResponse(const std::shared_ptr<UniversalState>& cmd) {
    nlohmann::json responsePacket, dataPacket, pumpNozzles, data;

    responsePacket["Id"] = packetId;
    responsePacket["Type"] = "PumpAuthorizeConfirmation";
    data["Pump"] = cmd->getId();
    data["Transaction"] = cmd->getDbId();
    responsePacket["Data"] = data;
    return responsePacket.dump();
}

std::string ProtocolSlaveJson::preparePacketStr(std::shared_ptr<UniversalState> cmd, const std::string &userName) {
    if(cmd->getControlUser() == "none") {
        cmd->setControlUser(userName);
    }
    switch(cmd->getType()) {
        case StateType::NOT_SERVED:
            return resolveNotServerResponse(cmd);
        case StateType::HANGED_FREE:
            return resolveHangedFreeResponse(cmd);
        case StateType::REMOVED_WAITING:
        case StateType::REMOVED_FREE:
        case StateType::WAITING_APPROPRIATE_PISTOL:
            return resolveRemovedResponse(cmd);
        case StateType::AUTHORIZE_REGISTERED:
            return resolveAuthorizeResponse(cmd);
        case StateType::FUEL_SUPPLY:
            return resolveFuelSupply(std::dynamic_pointer_cast<UniversalAmount>(cmd));
        case StateType::SUPPLY_DONE: {
            return resolveSupplyDone(std::dynamic_pointer_cast<UniversalAmount>(cmd));
        }
        case StateType::TOTAL_RESPONSE:
            return  resolveTotals(std::dynamic_pointer_cast<UniversalTotals>(cmd));
        case StateType::TRANSACTION_ClOSED:
        case StateType::SUPPLY_STOPPED:
            return resolveConfirmation();
        case StateType::TRANSACTION_UNClOSED:
            return resolveError("");
        default:
            LogPrintf(spdlog::level::warn, "|PTS| Undefined state response to create packet. Returning pump noy served");
            break;
    }
    return resolveNotServerResponse(cmd);
}

[[maybe_unused]] std::vector<UniversalCmd> ProtocolSlaveJson::parseMultipleCmd(const std::vector<unsigned char>& cmd) {
    std::vector<UniversalCmd> resCmd;
    auto cmdObj = nlohmann::json::parse(cmd);

//    for(auto& singleCmd : )
    return resCmd;
}
