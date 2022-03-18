#ifndef PTS2_0_METIATOR_H
#define PTS2_0_METIATOR_H
#include <iostream>
#include <string>
#include "UniversalCmd.h"
#include <memory>
#include <utility>
#include <UniversalState.h>
#include <UniversalTech.h>
#include <UniversalPrice.h>
#include <UniversalTotals.h>
#include <UniversalAuthorize.h>
#include <TechCmd.h>
#include <vector>
#include <map>
#include "nozzleLogicState.h"
#include <nlohmann/json.hpp>

/**
 * The Mediator interface declares a method used by components to notify the
 * mediator about various events. The Mediator may react to these events and
 * pass the execution to other components.
 */
class BaseComponent;

using columnId = int;

class Mediator {
public:
  virtual void checkUndoneTransactions(const nlohmann::json &cfg) const = 0;
  virtual void addCmd(std::unique_ptr<UniversalCmd> cmd) const = 0;
  virtual std::map<int, NozzleLogicState> checkUnfinishedKassaSimulations(const std::vector<int>& listOfPumps) const = 0;
  virtual std::unique_ptr<UniversalTech> operateCmd(std::unique_ptr<TechCmd> cmd) const = 0;
  virtual std::unique_ptr<UniversalCmd> jsonCheck(std::unique_ptr<UniversalCmd> cmd, const std::string &user) const = 0;
  virtual void fiscalCheck(std::unique_ptr<UniversalCmd> cmd) const = 0;
  virtual void checkDbUpdates(std::shared_ptr<UniversalState> data) const = 0;
  virtual std::shared_ptr<UniversalTotals> getKassaTotals(std::unique_ptr<UniversalCmd> cmd) const = 0;
  virtual void addKassaTotals(std::shared_ptr<UniversalState> data) const = 0;
  virtual void updateLicenseKet(const std::string updatedLicenseKey) const = 0;
  virtual std::string getLicenseKey() const = 0;
  virtual std::unique_ptr<UniversalCmd>
  checkKassaUpdates(std::unique_ptr<UniversalCmd> data, const std::string &user) const = 0;
  virtual FuelProduct getProductOnPumpNozzle(int pump, int nozzle) const = 0;
  virtual int gettransactionDbId(std::shared_ptr<UniversalState>& responseStae) const = 0;
  virtual std::unique_ptr<UniversalTech> setPrice(std::unique_ptr<UniversalPrice> cmd) const = 0;
  virtual void start() = 0;
  virtual void end() = 0;
  virtual void changeScenario(std::unique_ptr<UniversalCmd> cmd) const = 0;
  virtual void changeLog(std::map<columnId, bool> &&logLevelMap) const = 0;
  virtual std::shared_ptr<UniversalState> getState(int columnId) const = 0;
};

/**
 * The Base Component provides the basic functionality of storing a mediator's
 * instance inside component objects.
 */
class BaseComponent {
protected:
public:
    std::shared_ptr<Mediator> mediator_;

    BaseComponent() = default;
    void setMediator(std::shared_ptr<Mediator> tmpMediator) {
        mediator_ = std::move(tmpMediator);
    }
};

#endif
