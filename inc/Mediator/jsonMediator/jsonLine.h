#ifndef PTS2_0_JSONLINE_H
#define PTS2_0_JSONLINE_H

#include "Line.h"
#include <nlohmann/json.hpp>
#include <utility>

#include "JsonWorker.h"
#include "ProtocolSlaveJson.h"

#include "server_http.hpp"
#include <simple-websocket-server/server_ws.hpp>

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class JsonWorker;

class JsonLine : public Line, public std::enable_shared_from_this<JsonLine> {
private:
    std::shared_ptr<JsonWorker> jsonWorker{};
    std::unique_ptr<ProtocolSlaveJson> proto;
    HttpServer server;
    bool working = false;

    void setServerLambdas();
    explicit JsonLine() = default;
    explicit JsonLine(const nlohmann::json& cfgObj);
public:
    static std::shared_ptr<JsonLine> create(const nlohmann::json& cfgObj);
    void parseConfig(const nlohmann::json& cfg);
    void poll() override;
    std::future<void> start() override;

    [[maybe_unused]] void stop();

    void setJsonWorker(std::shared_ptr<JsonWorker> jsoner);
    static bool cmdIsTech(CmdType _type);
    bool checkUser(const std::string& authorizationHeader);
    static SimpleWeb::CaseInsensitiveMultimap emplaceHeader();
    static std::string getUserFromHeader(const std::string& authorizationHeader);
    void postJsonPtsCaller(const std::shared_ptr<HttpServer::Response> &response,
                           const std::shared_ptr<HttpServer::Request> &request);
    void optionCall(const std::shared_ptr<HttpServer::Response> &response,
                    const std::shared_ptr<HttpServer::Request> &request);

    [[maybe_unused]] void postCaller(const std::shared_ptr<HttpServer::Response> &response,
                    const std::shared_ptr<HttpServer::Request> &request);
    static void getCall(const std::shared_ptr<HttpServer::Response> &response,
                 const std::shared_ptr<HttpServer::Request> &request);
};

#endif //PTS2_0_JSONLINE_H
