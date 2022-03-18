#ifndef PTS2_0_JSONWORKER_H
#define PTS2_0_JSONWORKER_H

#include "Mediator.h"
#include "Line.h"
#include <map>
#include "jsonLine.h"
#include "KassaWorker.h"
#include "UniversalTech.h"
#include <server_http.hpp>

class JsonLine;

class JsonWorker : public BaseComponent, public std::enable_shared_from_this<JsonWorker> {
private:
    std::map<int, std::shared_ptr<JsonLine>> jsonLines;
    JsonWorker() = delete;
    std::map<std::string, std::string> users;
    explicit JsonWorker(const nlohmann::json& cfgJson);
    bool traceLogsActive = false;
public:
    void start();
    bool getTraceActivity() const;
    bool checkUserPass(const std::string& loginStr, const std::string& passStr);
    static std::shared_ptr<JsonWorker> create(const nlohmann::json& jsonObj);
    void parseConfig(const nlohmann::json& cfgObj);
    std::shared_ptr<UniversalState> processCmd(std::unique_ptr<UniversalCmd> cmd);
    std::unique_ptr<UniversalTech> processTechCmd(std::unique_ptr<UniversalCmd> cmd, const std::string &user);
};


#endif //PTS2_0_JSONWORKER_H
