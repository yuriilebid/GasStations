#ifndef PTS2_0_KASSAWORKER_H
#define PTS2_0_KASSAWORKER_H

#include <nlohmann/json.hpp>

#include "Mediator.h"
#include "Scenario.h"
#include "TechCmd.h"
#include "JsonWorker.h"
#include "Line.h"

class FiscalLine;

class KassaWorker : public BaseComponent, public std::enable_shared_from_this<KassaWorker> {
private:
    std::unordered_map<std::string, std::unique_ptr<Scenario>> scenarios;
    bool workingCondition;
    LogicType logicType;
    std::vector<std::shared_ptr<FiscalLine>> kassaLines;
    explicit KassaWorker(const nlohmann::json &cfgObj);
public:
    KassaWorker() = default;
    void checkUndoneTransactions(const nlohmann::json &cfg);
    static std::shared_ptr<KassaWorker> create(const nlohmann::json& cfgObj);
    std::shared_ptr<UniversalState> processCmd(std::unique_ptr<UniversalCmd> cmd, const std::string &fiscalUser);
    void changeLogicType(std::unique_ptr<TechCmd> cmd);
    void parseCfg(const nlohmann::json &cfgObj);
    std::vector<std::future<void>> start();
};

using columnId = int;
using columnAddr = int;

class FiscalLine : public Line, public std::enable_shared_from_this<FiscalLine> {
private:
    std::shared_ptr<KassaWorker> fiscalWorker{};
    std::unique_ptr<ProtocolSlave> proto{};
    FiscalLine() = default;
    bool pollingCondition{};
    std::string user;
    FiscalType type;

    explicit FiscalLine(const nlohmann::json& cfgObj, std::shared_ptr<KassaWorker> fiscaler);
public:
    static std::shared_ptr<FiscalLine> create(const nlohmann::json& cfgObj, const std::shared_ptr<KassaWorker>& fiscaler);
    void parseConfig(const nlohmann::json& cfgobj);
    std::future<void> start() override;
    void poll() override;
};

#endif
