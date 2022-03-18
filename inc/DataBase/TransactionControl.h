#ifndef PTS2_0_TRANSACTIONCONTROL_H
#define PTS2_0_TRANSACTIONCONTROL_H

#include <sqlite3.h>
#include <memory>
#include "UniversalAuthorize.h"
#include "Mediator.h"
#include <UniversalState.h>
#include <UniversalTotals.h>
#include "NozzleLogicState.h"
#include <map>
#include <vector>
#include "FuelProduct.h"

class TransactionControl : public BaseComponent {
private:
    sqlite3 *db {};
    bool dbOpened {};
    std::string dbPath = "./DB";
    std::map<int, std::string> pumpIdfuelType;

    int getLastStateOnTransaction(int pump);
public:
    TransactionControl();
    int openDB();
    void initTables();
    void closeDb();

    std::string getLastConfig();
    int getTransactionId(const std::shared_ptr<UniversalState>& data);
    void addProduct(const std::shared_ptr<FuelProduct>& fuelUnit);
    void updateProduct(const std::shared_ptr<FuelProduct>& fuelUnit);
    void addConfigurations(const std::string& cfgStr);
    void addTotalKassaInfo(const std::shared_ptr<UniversalTotals> &data);
    float getLastConfigVersion();
    std::string getLicenseKey();
    void updateLicenseKey(const std::string& updatedLicenseKey);
    std::map<int, NozzleLogicState> checkUnfinishedSimulations(const std::vector<int>& listOfPumps);
    std::shared_ptr<FuelProduct> getProductInfo(int productId);
    std::shared_ptr<UniversalTotals> getKassaTotals(std::unique_ptr<UniversalCmd> cmd);
    std::unique_ptr<UniversalCmd> updateTransactionFromCmd(std::unique_ptr<UniversalCmd> data);
    std::unique_ptr<UniversalCmd> addTransaction(std::unique_ptr<UniversalCmd> data, const std::string &user);
    void updateTransaction(const std::shared_ptr<UniversalState>& data);

    std::unique_ptr<UniversalCmd> addSimulation(std::unique_ptr<UniversalCmd> data, const std::string &user);
    std::unique_ptr<UniversalCmd> updateSimulationFromCmd(std::unique_ptr<UniversalCmd> data, const std::string &user);
};

#endif //PTS2_0_TRANSACTIONCONTROL_H
