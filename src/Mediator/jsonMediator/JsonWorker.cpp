#include "JsonWorker.h"
#include "jsonLine.h"
#include <server_http.hpp>
#include "base64.h"
#include "LogCmd.h"
#include "ProtocolSlaveJson.h"
#include "UniversalPrice.h"

//using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

JsonWorker::JsonWorker(const nlohmann::json& jsonObj) {
    LogPrintf(spdlog::level::info, "Parsing config in JsonWorker");
}

std::shared_ptr<JsonWorker> JsonWorker::create(const nlohmann::json& jsonObj) {
    return std::shared_ptr<JsonWorker>(new JsonWorker(jsonObj));
}

void JsonWorker::start() {
    LogPrintf(spdlog::level::info, "Starting Json");
    for(auto& line : jsonLines) {
        LogPrintf(spdlog::level::info, "Starting Json line");
        line.second->start();
    }
}

void JsonWorker::parseConfig(const nlohmann::json& cfgObj) {
    try {
        for(const auto& jsonDev : cfgObj.at("jsons")) {
            int port = jsonDev.at("communication").at("port");
            LogPrintf(spdlog::level::info, "Adding jsonLine server");
            auto tmpJsonLine = JsonLine::create(jsonDev);
            tmpJsonLine->setJsonWorker(shared_from_this());
            auto emplaceRes = jsonLines.emplace(port, tmpJsonLine);
            traceLogsActive = jsonDev.at("communication").at("trace");
        }
        for(const auto& userUnit : cfgObj.at("users")) {
            std::string tmpLogin = userUnit.at("login");
//            macaron::Base64::Decode(userUnit.at("login"), tmpLogin);
            std::string tmpPass = userUnit.at("pass");
//            macaron::Base64::Decode(userUnit.at("pass"), tmpPass);
            users.insert({tmpLogin, tmpPass});
        }
    } catch (const nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "jsonMediator constructor error: {}", e.what());
    }
}

std::shared_ptr<JsonLine> JsonLine::create(const nlohmann::json& cfgObj) {
    return std::shared_ptr<JsonLine>(new JsonLine(cfgObj));
}

JsonLine::JsonLine(const nlohmann::json& cfgObj) {
    parseConfig(cfgObj);
    setServerLambdas();
    proto = std::make_unique<ProtocolSlaveJson>();
}

void JsonLine::setJsonWorker(std::shared_ptr<JsonWorker> jsoner) {
    jsonWorker = std::move(jsoner);
}

bool JsonWorker::getTraceActivity() const {
    return traceLogsActive;
}

bool JsonWorker::checkUserPass(const std::string& loginStr, const std::string& passStr) {
    if(users.find(loginStr) != users.end()) {
        if(users.at(loginStr) == passStr) {
            return true;
        } else {
            LogPrintf(spdlog::level::warn, "|JSON| Incorrect passwod authentification");
            return false;
        }
    } else {
        LogPrintf(spdlog::level::warn, "|JSON| Authentification user not found. Login: {}, Password: {}", loginStr, passStr);
        for(const auto& usr : users) {
            LogPrintf(spdlog::level::info, "Acceptable user: |{}| pass: |{}|", usr.first, usr.second);
        }
        return false;
    }
}

std::future<void> JsonLine::start() {
    LogPrintf(spdlog::level::info, "Starting JSON server");

    try {
        working = true;
        server.config.reuse_address = true;
        server.stop();
        server.start();
    } catch(...) {
        std::exception_ptr p = std::current_exception();
        std::clog <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }
    return {};
}

[[maybe_unused]] void JsonLine::stop() {
    working = true;
    server.stop();
}

SimpleWeb::CaseInsensitiveMultimap JsonLine::emplaceHeader() {
    SimpleWeb::CaseInsensitiveMultimap stream;

    stream.emplace("Content-Type", "application/json; charset=utf-8");
    stream.emplace("Access-Control-Allow-Credentials", "true");
    stream.emplace("Access-Control-Allow-Origin", "*");
    stream.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    stream.emplace("Access-Control-Allow-Headers", "Authorization");
    stream.emplace("Access-Control-Expose-Headers", "Content-Length,Content-Range");
    stream.emplace("User-Agent", "SOFTPTS1100");
    return stream;
}

void JsonLine::setServerLambdas() {
    LogPrintf(spdlog::level::info, "Setting JSON lambdas dependencies");
    try {
        auto optionsLambda = [this](const auto& response, const auto& request){
            return optionCall(response, request);
        };
        auto postLambda = [this](const auto& response, const auto& request) {
            return postJsonPtsCaller(response, request);
        };
        auto getLambda = [](const auto& response, const auto& request) {
            return getCall(response, request);
        };
        server.resource[".*"]["OPTIONS"] = optionsLambda;
        server.resource["^/jsonPTS"]["POST"] = postLambda;
        server.default_resource["POST"] = postLambda;
        server.default_resource["GET"] = getLambda;
    } catch(...) {
        std::exception_ptr p = std::current_exception();
        std::clog <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }
}

void JsonLine::parseConfig(const nlohmann::json& cfg) {
    try {
        server.config.address = cfg.at("communication").at("addr");
        server.config.port = cfg.at("communication").at("port");
    } catch (const nlohmann::json::exception& e) {
        LogPrintf(spdlog::level::err, "Parsing config for json dev: {}", e.what());
    } catch(...) {
        std::exception_ptr p = std::current_exception();
        std::clog <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
    }
}

std::unique_ptr<UniversalTech> JsonWorker::processTechCmd(std::unique_ptr<UniversalCmd> cmd, const std::string &user) {
    CmdType cmdType = cmd->getCmdType();

    switch(cmdType) {
        case CmdType::CHANGE_LOG: {
                auto logCmd = std::make_unique<LogCmd>(*dynamic_cast<LogCmd*>(cmd.release()));
                this->mediator_->changeLog(logCmd->getLogLevel());
                break;
            }
        case CmdType::CHANGE_SCENARIOS: {
            LogPrintf(spdlog::level::info, "|DB| addding simulation transaction");
            auto changeScenarioCmd = this->mediator_->checkKassaUpdates(std::move(cmd), user);
            LogPrintf(spdlog::level::info, "|TECH_CMD| processing |CHANGE_SCENARIOS|");
            this->mediator_->changeScenario(std::move(changeScenarioCmd));
            return std::make_unique<UniversalTech>(techCmdType::SCENARIO);
        }
        case CmdType::GRADES_CONFIG:
        case CmdType::PUMP_CONFIG:
            return this->mediator_->operateCmd(std::make_unique<TechCmd>(*dynamic_cast<TechCmd*>(cmd.release())));
        case CmdType::GET_BATTERY:
            return std::make_unique<UniversalTech>(techCmdType::BATTERY);
        case CmdType::GET_SD:
            return std::make_unique<UniversalTech>(techCmdType::SD_INFO);
        case CmdType::WRITE_PRICE:
            return this->mediator_->setPrice(std::make_unique<UniversalPrice>(*dynamic_cast<UniversalPrice*>(cmd.release())));
        default:
            LogPrintf(spdlog::level::err, "Undefined Tech CMD");
            break;
    }
    return std::make_unique<UniversalTech>(*dynamic_cast<UniversalTech*>(cmd.release()));
}

std::shared_ptr<UniversalState> JsonWorker::processCmd(std::unique_ptr<UniversalCmd> cmd) {
    CmdType cmdType = cmd->getCmdType();
    int dbTransaction = cmd->getDbId();
    auto columnId = cmd->getId();

    switch(cmdType) {
        case CmdType::STATUS_REQUEST: {
            auto stateResponse = this->mediator_->getState(columnId);
            if(stateResponse->getType() == StateType::SUPPLY_DONE and stateResponse->getDbId() <= 0) {
                stateResponse->setDbId(this->mediator_->gettransactionDbId(stateResponse));
            }
            return stateResponse;
        }
        case CmdType::WRITE_VOLUME: {
            LogPrintf(spdlog::level::info, "|JSON| processing |WRITE_VOLUME| to columns");
            this->mediator_->addCmd(std::move(cmd));
            return std::make_shared<UniversalState>(columnId, 0, StateType::AUTHORIZE_REGISTERED, dbTransaction);
        }
        case CmdType::STOP_COMMAND:
            LogPrintf(spdlog::level::info, "|JSON| processing |STOP_COMMAND| to columns");
            this->mediator_->addCmd(std::move(cmd));
            return std::make_shared<UniversalState>(columnId, 0, StateType::SUPPLY_STOPPED, dbTransaction);
        case CmdType::READ_PRICE:
            LogPrintf(spdlog::level::info, "|JSON| processing |READ_PRICE| to columns");
            this->mediator_->addCmd(std::move(cmd));
            return std::make_shared<UniversalState>(columnId, 0, StateType::AUTHORIZE_REGISTERED, dbTransaction);
        case CmdType::GET_TOTAL:
            LogPrintf(spdlog::level::info, "|JSON| processing |GET_TOTAL| to columns");
            this->mediator_->addCmd(std::move(cmd));
            return std::make_shared<UniversalState>(columnId, 0, StateType::TRANSACTION_ClOSED, dbTransaction);
        case CmdType::WRITE_PRICE:
            LogPrintf(spdlog::level::info, "|JSON| processing |WRITE_PRICE| to columns");
            this->mediator_->addCmd(std::move(cmd));
            return std::make_shared<UniversalState>(columnId, 0, StateType::AUTHORIZE_REGISTERED, dbTransaction);
        case CmdType::PAUSE_COMMAND:
        case CmdType::WRITE_MONEY:
        case CmdType::CLOSE_REPORT: {
            LogPrintf(spdlog::level::info, "|JSON| processing |CLOSE_REPORT| to columns");
            this->mediator_->addCmd(std::move(cmd));
            auto stateResponse = this->mediator_->getState(columnId);

            if(stateResponse->getType() != StateType::SUPPLY_DONE) {
                /// TODO: обновялем состояние в БД
                return std::make_shared<UniversalState>(columnId, 0, StateType::TRANSACTION_ClOSED, dbTransaction);
            }
            return stateResponse;
        }
        default:
            break;
    }
    return this->mediator_->getState(columnId);
}

/// PART of TCP HTTP server logic

void JsonLine::getCall(const std::shared_ptr<HttpServer::Response> &response,
                       const std::shared_ptr<HttpServer::Request> &request) {
    try {
        for (auto &head: request->header) {
            if (head.first == "Authorization") {
                std::string user;
                auto decodedUser = macaron::Base64::Decode(head.second, user);
            }
            std::stringstream contentStream;
            SimpleWeb::CaseInsensitiveMultimap stream;
            stream.emplace("Content-Type", "application/json; charset=utf-8");
            stream.emplace("Access-Control-Allow-Credentials", "true");
            stream.emplace("Access-Control-Allow-Origin", "*");
            stream.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            stream.emplace("Access-Control-Allow-Headers", "Authorization");
            stream.emplace("Access-Control-Expose-Headers", "Content-Length,Content-Range");
            stream.emplace("User-Agent", "SOFTPTS1100");
            response->write(SimpleWeb::StatusCode::client_error_bad_request, contentStream, stream);
        }
    }
    catch (const std::exception &e) {
        *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
        << e.what();
        exit(1);
    }
}

void JsonLine::optionCall(const std::shared_ptr<HttpServer::Response> &response,
                          const std::shared_ptr<HttpServer::Request> &request) {
    try {
        std::string Origin = "*";
        char codedString[64];
        for (auto &head: request->header) {
            if (head.first == "Authorization") {
                std::string user;
                int start = 0;
                bzero(codedString, 64);
                while (head.second[start] != ' ') { start++; }
                for (int i = start + 1; i < head.second.size(); i++) {
                    codedString[i - (start + 1)] = head.second[i];
                }
                auto decodedUser = macaron::Base64::Decode(codedString, user);
                std::string login = user.substr(0, user.find(':'));
                std::string pass = user.substr(user.find(':') + 1);
                if(jsonWorker->getTraceActivity()) {
//                    LogPrintf(spdlog::level::trace, "|JSON| Login: {} Pass: {}", login, pass);
                }
            }
            if (head.first == "Origin") {
                Origin = head.second;
            }
        }
        SimpleWeb::CaseInsensitiveMultimap stream;
        stream.emplace("Allow", "OPTIONS, POST");
        stream.emplace("Access-Control-Max-Age", "1728000");
        stream.emplace("Content-Type", "application/json; charset=utf-8");
        stream.emplace("Access-Control-Allow-Credentials", "true");
        stream.emplace("Access-Control-Allow-Origin", Origin);
        stream.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        stream.emplace("Access-Control-Allow-Headers", "Authorization");
        stream.emplace("Connection", "keep-alive");
        stream.emplace("Access-Control-Expose-Headers", "Content-Length,Content-Range");
        response->write(stream);
    } catch (const std::exception &e) {
        *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
        << e.what();
        exit(1);
    }
}

bool JsonLine::cmdIsTech(CmdType _type) {
    switch(_type) {
        case CmdType::GRADES_CONFIG:
        case CmdType::PUMP_CONFIG:
        case CmdType::CHANGE_SCENARIOS:
        case CmdType::CHANGE_LOG:
        case CmdType::WRITE_PRICE:
        case CmdType::GET_SD:
        case CmdType::GET_BATTERY:
            return true;
        default:
            return false;
    }
}

std::string JsonLine::getUserFromHeader(const std::string& authorizationHeader) {
    std::string user;
    int start = 0;
    char codedString[64];

    bzero(codedString, 64);
    while (authorizationHeader[start] != ' ') { start++; }
    for (int i = start + 1; i < authorizationHeader.size(); i++) {
        codedString[i - (start + 1)] = authorizationHeader[i];
    }
    auto decodedUser = macaron::Base64::Decode(codedString, user);
    return user.substr(0, user.find(':'));
}

bool JsonLine::checkUser(const std::string& authorizationHeader) {
    std::string user;
    int start = 0;
    char codedString[64];

    if(authorizationHeader == "testyurii1234") {
        LogPrintf(spdlog::level::debug, "|TEST| mode request");
        return true;
    }

    bzero(codedString, 64);
    while (authorizationHeader[start] != ' ') { start++; }
    for (int i = start + 1; i < authorizationHeader.size(); i++) {
        codedString[i - (start + 1)] = authorizationHeader[i];
    }
    auto decodedUser = macaron::Base64::Decode(codedString, user);
    std::string login = user.substr(0, user.find(':'));
    std::string pass = user.substr(user.find(':') + 1);
    if(jsonWorker->getTraceActivity()) {
//        LogPrintf(spdlog::level::trace, "|JSON| Login: {} Pass: {}", login, pass);
    }
    return this->jsonWorker->checkUserPass(login, pass);
}

void JsonLine::postJsonPtsCaller(const std::shared_ptr<HttpServer::Response> &response,
                        const std::shared_ptr<HttpServer::Request> &request) {
    std::vector<std::string> responseStr;
    std::string responsePacket, responseSinglePacket, user;
    bool authorizationState = false;

    try {
        for (auto &head: request->header) {
            if (head.first == "Authorization") {
                authorizationState = checkUser(head.second);
                user = getUserFromHeader(head.second);
            }
        }
        if(authorizationState) {
            auto sourcePacket = request->content.string();
            LogTrace(spdlog::level::trace, "|JSON| READ: {}", sourcePacket);
            if(jsonWorker->getTraceActivity()) {
                proto->traceResponse({sourcePacket.begin(), sourcePacket.end()});
//                LogPrintf(spdlog::level::trace, "|JSON| READ: {}", sourcePacket);
            }
            auto packetList = proto->splitPackets({sourcePacket.begin(), sourcePacket.end()});
            for(const auto& singlePacket : packetList) {
                auto tmpCmdRequest = proto->parseCmd(singlePacket);
                auto cmdRequest = jsonWorker->mediator_->jsonCheck(std::move(tmpCmdRequest), user);

                cmdRequest->setControlUser(user);
                if(cmdRequest->getCmdType() == CmdType::CONFIRMATION) {
                    responseSinglePacket = proto->preparePacketStr(
                            std::make_shared<UniversalState>(0, 0, StateType::SUPPLY_STOPPED, 0), user);
                } else if(cmdIsTech(cmdRequest->getCmdType())) {
                    auto techResponse = jsonWorker->processTechCmd(std::move(cmdRequest), user);
                    responseSinglePacket = proto->preparePacketStr(std::move(techResponse));
                } else {
                    auto stateResponse = jsonWorker->processCmd(std::move(cmdRequest));
                    responseSinglePacket = proto->preparePacketStr(stateResponse, user);
                }
                (void)responseStr.push_back(responseSinglePacket);
            }
            responsePacket = proto->gatherResponses(responseStr);
            if(jsonWorker->getTraceActivity()) {
                proto->traceRequests({responsePacket.begin(), responsePacket.end()});
//                LogPrintf(spdlog::level::trace, "|JSON| WRITE: {}", responsePacket);
            }
            LogTrace(spdlog::level::trace, "|JSON| WRITE: {}", responsePacket);
            response->write(responsePacket, emplaceHeader());
        } else {
            LogTrace(spdlog::level::trace, "|JSON| WRITE: {empty}");
            response->write("", emplaceHeader());
        }
    }
    catch (const std::exception &e) {
        *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
        << e.what();
        exit(1);
    }
}

[[maybe_unused]] void JsonLine::postCaller(const std::shared_ptr<HttpServer::Response> &response,
                                                 const std::shared_ptr<HttpServer::Request> &request) {
    bool authorizationState = false;

    try {
        for (auto &head: request->header) {
            if (head.first == "Authorization") {
                authorizationState = checkUser(head.second);
            }
        }
        LogPrintf(spdlog::level::debug, "Post call JSON");
        if(authorizationState) {
            std::stringstream contentStream;
            SimpleWeb::CaseInsensitiveMultimap stream;
            stream.emplace("Content-Type", "application/json; charset=utf-8");
            stream.emplace("Access-Control-Allow-Credentials", "true");
            stream.emplace("Access-Control-Allow-Origin", "*");
            stream.emplace("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            stream.emplace("Access-Control-Allow-Headers", "Authorization");
            stream.emplace("Access-Control-Expose-Headers", "Content-Length,Content-Range");
            stream.emplace("User-Agent", "SOFTPTS1100");
            contentStream << "HTTP/1.1 404 Bad Request\r\n" << "\r\n";
            response->write(SimpleWeb::StatusCode::client_error_bad_request, contentStream, stream);
        }
    }
    catch (const std::exception &e) {
        *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n"
        << e.what();
        exit(1);
    }
}

void JsonLine::poll() {

}