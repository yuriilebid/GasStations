#ifndef PTS2_0_CONCREATEMEDIATOR_H
#define PTS2_0_CONCREATEMEDIATOR_H

#include "Mediator.h"
#include "ColumnWorker.h"
#include "KassaWorker.h"
#include "JsonWorker.h"
#include "wamp.h"
#include "TransactionControl.h"

class ConcreteMediator : public Mediator, public std::enable_shared_from_this<ConcreteMediator> {
 private:
  std::unique_ptr<ColumnWorker> columnWorker;
//  std::unique_ptr<Wamp> wampWorker;
  std::shared_ptr<JsonWorker> jsonWorker;
  std::shared_ptr<KassaWorker> kassaWorker;
  std::shared_ptr<TransactionControl> dataBase;
  std::vector<std::future<void>> futuresList;

  explicit ConcreteMediator(const nlohmann::json& cfgObj);
 public:
  void addCmd(std::unique_ptr<UniversalCmd> cmd) const override;
  void checkUndoneTransactions(const nlohmann::json &cfg) const override;
  std::map<int, NozzleLogicState> checkUnfinishedKassaSimulations(const std::vector<int>& listOfPumps) const override;
  std::unique_ptr<UniversalTech> operateCmd(std::unique_ptr<TechCmd> cmd) const override;
  std::unique_ptr<UniversalTech> setPrice(std::unique_ptr<UniversalPrice> cmd) const override;
  void checkDbUpdates(std::shared_ptr<UniversalState> data) const override;
  std::shared_ptr<UniversalTotals> getKassaTotals(std::unique_ptr<UniversalCmd> cmd) const override;
  void addKassaTotals(std::shared_ptr<UniversalState> data) const override;
  std::unique_ptr<UniversalCmd> checkKassaUpdates(std::unique_ptr<UniversalCmd> data, const std::string &user) const override;
  void setAllMediator();
  std::unique_ptr<UniversalCmd> jsonCheck(std::unique_ptr<UniversalCmd> cmd, const std::string &user) const override;
  int gettransactionDbId(std::shared_ptr<UniversalState>& responseStae) const override;
  void fiscalCheck(std::unique_ptr<UniversalCmd> cmd) const override;
  FuelProduct getProductOnPumpNozzle(int pump, int nozzle) const override;
  static std::shared_ptr<ConcreteMediator> create(const nlohmann::json& cfgObj);
  void start() override;
  void end() override;
  void changeScenario(std::unique_ptr<UniversalCmd> cmd) const override;
  void changeLog(std::map<columnId, bool> &&logLevelMap) const override;
  void updateLicenseKet(const std::string updatedLicenseKey) const override;
  std::string getLicenseKey() const override;
  ~ConcreteMediator();
  std::shared_ptr<UniversalState> getState(int columnId) const override;
};

#endif //PTS2_0_CONCREATEMEDIATOR_H
