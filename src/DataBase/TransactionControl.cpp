#include "DataBase/TransactionControl.h"
#include "logger.h"
#include "UniversalState.h"
#include "UniversalAmount.h"
#include "nlohmann/json.hpp"
#include <dirent.h>

int TransactionControl::openDB() {
    DIR *dir;
    struct dirent *entry;
    bool directoryFound = false;
    const std::string dbDirName("DB");

    try {
        std::vector<spdlog::sink_ptr> sinks;
        std::vector<spdlog::sink_ptr> fileSinks;
        std::vector<spdlog::sink_ptr> consoleSinks;

        dir = opendir(".");
        if (!dir) {
            perror("diropen");
            exit(1);
        }
        while ( (entry = readdir(dir)) != nullptr && !directoryFound) {
            if(entry->d_name == dbDirName) {
                directoryFound = true;
            }
        }
        if(!directoryFound) {
            LogPrintf(spdlog::level::warn, "|PTS| Directory |\"DB\"| do not exists. Creating new one");
            int check = mkdir(dbDirName.c_str(), 0777);
            if(check != 0) {
                LogPrintf(spdlog::level::err, "|PTS| Cannot create directory |\"DB\"|");
                exit(1);
            }
            else {
                LogPrintf(spdlog::level::info, "|PTS| Directory |\"DB\"| created successfully");
            }
        }
    }
    catch (const spdlog::spdlog_ex &e) {
        std::cout << "Error opening log: " << e.what() << std::endl;
        return -1;
    }

    int rc;
    LogPrintf(spdlog::level::info, "OpeninDB func entry");
    std::string dbPathFile = "./DB/data.sqlite";
    LogPrintf(spdlog::level::info, "Opening dataBase: {}", dbPathFile);

    rc = sqlite3_open(dbPathFile.c_str(), &db);
    if(rc) {
        LogPrintf(spdlog::level::err, "|DB| Cannot open database");
        if(sqlite3_close(db)) {
            LogPrintf(spdlog::level::err, "|DB| Cannot close db file");
        }
        return -1;
    }
    dbOpened = true;
    return 0;
}

void TransactionControl::closeDb() {
    sqlite3_close(db);
}

std::string getTimeNow() {
    char buffer[80];
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%m-%Y/%H-%M-%S",timeinfo);
    std::string str(buffer);
    str.replace(str.find('/'), 1, " ");
    return str;
}

int TransactionControl::getTransactionId(const std::shared_ptr<UniversalState>& data) {
    sqlite3_stmt* stmt;
    auto sql = fmt::format(R"(SELECT id FROM transactions WHERE pump = {} ORDER BY id DESC LIMIT 1;")",
                           data->getId());
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    int pumpId = sqlite3_column_int(stmt, 0);
    LogFile(spdlog::level::info, "Found transaction with Id {}", pumpId);
    sqlite3_finalize(stmt);
    return pumpId;
}

void TransactionControl::initTables() {
    std::string sql;
    char *errMsg;
    int rc;

    if(!dbOpened) {
        return;
    }
    sql = "CREATE TABLE IF NOT EXISTS transactions ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "startdate TEXT NOT NULL DEFAULT (datetime('now')), "
          "enddate TEXT NOT NULL, "
          "state INT NOT NULL DEFAULT 1, "
          "pump INT NOT NULL, "
          "nozzle INT NOT NULL, "
          "fueltype TEXT NOT NULL, "
          "price INT NOT NULL, "
          "volume INT NOT NULL DEFAULT 0, "
          "sum INT NOT NULL DEFAULT 0, "
          "requestedvolume INT NOT NULL DEFAULT 0, "
          "requestedsum INT NOT NULL DEFAULT 0, "
          "user TEXT NOT NULL,"
          "trktxnid INT NOT NULL DEFAULT 0);";

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if(rc) {
        LogPrintf(spdlog::level::err, "|DB| Cannot create table for transaction: {}", errMsg);
        exit(1);
    }

    sql = "CREATE TABLE IF NOT EXISTS simulations ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "startdate TEXT NOT NULL DEFAULT (datetime('now')), "
          "enddate TEXT NOT NULL, "
          "state INT NOT NULL DEFAULT 1, "
          "pump INT NOT NULL, "
          "nozzle INT NOT NULL, "
          "fueltype TEXT NOT NULL, "
          "price INT NOT NULL, "
          "volume INT NOT NULL DEFAULT 0, "
          "sum INT NOT NULL DEFAULT 0, "
          "requestedvolume INT NOT NULL DEFAULT 0, "
          "requestedsum INT NOT NULL DEFAULT 0, "
          "user TEXT NOT NULL,"
          "trktxnid INT NOT NULL DEFAULT 0);";

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if(rc) {
        LogPrintf(spdlog::level::err, "|DB| Cannot create table for simulations: {}", errMsg);
        exit(1);
    }

    sql = "CREATE TABLE IF NOT EXISTS Config ("
          "version REAL PRIMARY KEY, "
          "dateadded TEXT NOT NULL DEFAULT (datetime('now')), "
          "config TEXT NOT NULL);";

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if(rc) {
        LogPrintf(spdlog::level::err, "|DB| Cannot create table for config: {}", errMsg);
        exit(1);
    }

    sql = "CREATE TABLE IF NOT EXISTS prices (productid INT PRIMARY KEY NOT NULL, "
          "productname TEXT NOT NULL,"
          "productprice INT NOT NULL,"
          "productexpanssion REAL NOT NULL);";

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if(rc) {
        LogPrintf(spdlog::level::err, "|DB| Cannot create price config table");
        exit(1);
    }

     sql = "CREATE TABLE IF NOT EXISTS totals (id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "measure_date TEXT NOT NULL DEFAULT (datetime('now')), "
          "column INT NOT NULL, "
          "nozzle INT NOT NULL, "
          "product_id INT NOT NULL, "
          "volume INT NOT NULL);";

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if(rc) {
        LogPrintf(spdlog::level::err, "|DB| Cannot create price config table");
        exit(1);
    }

    sql = "CREATE TABLE IF NOT EXISTS settings ( "
          "date_added DATETIME NOT NULL DEFAULT(CURRENT_TIMESTAMP), "
          "id INTEGER PRIMARY KEY, "
          "key TEXT, "
          "value TEXT);";

    rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if(rc) {
        LogPrintf(spdlog::level::err, "|DB| Cannot create price config table");
        exit(1);
    }
}

std::string TransactionControl::getLastConfig() {
    sqlite3_stmt* stmt;
    std::string actualCfg;
    auto query = fmt::format(R"(SELECT * FROM Config WHERE version = (SELECT MAX(version) FROM Config);)");
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    if((char*)sqlite3_column_text(stmt, 2) != nullptr) {
        actualCfg = (char *) sqlite3_column_text(stmt, 2);
        LogPrintf(spdlog::level::info, "|PTS| Actual config: {}", actualCfg);
    }
    sqlite3_finalize(stmt);
    return actualCfg;
}

std::map<int, NozzleLogicState> TransactionControl::checkUnfinishedSimulations(const std::vector<int>& listOfPumps) {
    sqlite3_stmt* stmt;
    std::map<int, NozzleLogicState> undoneSimulations;
    LogPrintf(spdlog::level::info, "|PTS| Checking for unfinished transactions");

    for(auto& pumpId : listOfPumps) {
        LogPrintf(spdlog::level::info, "|PTS| Checking for unfinished transaction for pump |{}|", pumpId);
        auto query = fmt::format(R"(SELECT * FROM simulations WHERE pump = {} AND state < 3;)", pumpId);
        sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        if((char*)sqlite3_column_text(stmt, 1) != nullptr) {
            int activeNozzle = sqlite3_column_int(stmt, 5);
            int requestedVolumeInMl = sqlite3_column_int(stmt, 10);
            int price = sqlite3_column_int(stmt, 7);
            int state = sqlite3_column_int(stmt, 3);

            if(state == 2) {
                LogPrintf(spdlog::level::info, "|PTS| Setting State |AUTHORIZED| on pump |{}|", pumpId);
                NozzleLogicState undoneSingleTransaction(activeNozzle, requestedVolumeInMl, price,
                                                         ScenarioLastState::AUTHORIZED, LogicType::SLEEP);
                undoneSimulations[pumpId] = undoneSingleTransaction;
            } else {
                LogPrintf(spdlog::level::info, "|PTS| Setting State |IDLE| on pump |{}|", pumpId);
                NozzleLogicState undoneSingleTransaction(activeNozzle, requestedVolumeInMl, price,
                                                         ScenarioLastState::IDLE, LogicType::SLEEP);
                undoneSimulations[pumpId] = undoneSingleTransaction;
            }
        }
        sqlite3_finalize(stmt);
    }
    return undoneSimulations;
}

float TransactionControl::getLastConfigVersion() {
    sqlite3_stmt* stmt;
    float actuaclVersion = 0;
    auto query = fmt::format(R"(SELECT MAX(version) FROM Config;)");
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    if((float)sqlite3_column_double(stmt, 0)) {
        actuaclVersion = (float)sqlite3_column_double(stmt, 0);
        LogPrintf(spdlog::level::info, "|PTS| Actual config version: {}", actuaclVersion);
    }
    sqlite3_finalize(stmt);
    return actuaclVersion;
}

std::string TransactionControl::getLicenseKey() {
    sqlite3_stmt* stmt;
    std::string licenseKey {};
    auto query = fmt::format(R"(SELECT key FROM settings;)");
    
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    if((char*)sqlite3_column_text(stmt, 0)) {
        licenseKey = (char*)sqlite3_column_text(stmt, 0);
        LogPrintf(spdlog::level::info, "|PTS| Actual license key: {}", licenseKey);
    }
    sqlite3_finalize(stmt);
    return licenseKey;
}

void TransactionControl::updateLicenseKey(const std::string& updatedLicenseKey) {
    sqlite3_stmt* stmt;
    char* errMsg;
    auto query = fmt::format(R"(UPDATE settings SET key = '%s';)", updatedLicenseKey);
    auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        LogPrintf(spdlog::level::err, "|DB| Cannot update settings: {}", errMsg);
        sqlite3_free(errMsg);
    } else {
        LogPrintf(spdlog::level::info, "|DB||UPDATE| license key to: {}", updatedLicenseKey);
    }
    sqlite3_finalize(stmt);
}

void TransactionControl::addConfigurations(const std::string &cfgStr) {
    char* errMsg = nullptr;
    std::string query;
    auto cfgObj = nlohmann::json::parse(cfgStr);
    float cfgVersion = cfgObj.at("version");
    float actualVersion = getLastConfigVersion();

    if(cfgVersion > actualVersion) {
        query = fmt::format(
                R"(INSERT INTO Config (version, dateadded, config) VALUES({}, '{}', '{}') ON CONFLICT(version) DO UPDATE SET config = '{}';)",
                cfgVersion, getTimeNow().c_str(), cfgStr, cfgStr);
        auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            LogPrintf(spdlog::level::err, "|DB| Cannot insert Config: {}", errMsg);
            sqlite3_free(errMsg);
        } else {
            LogPrintf(spdlog::level::info, "|DB| Inserted config failed: {}", errMsg);
        }
    }
}

int TransactionControl::getLastStateOnTransaction(int pump) {
    sqlite3_stmt* stmt;
    std::string actucalCfg;
    int stateId = 0;

    auto query = fmt::format(R"(SELECT state FROM transactions WHERE id = (SELECT MAX(id) FROM transactions WHERE pump = {});)", pump);
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    if(stmt != nullptr) {
        stateId = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return stateId;
    } else {
        sqlite3_finalize(stmt);
        return stateId;
    }
}

std::unique_ptr<UniversalCmd>
TransactionControl::addTransaction(std::unique_ptr<UniversalCmd> data, const std::string &user) {
    auto authorizeCmd = dynamic_unique_cast<UniversalAuthorize>(std::move(data));
    int pumpId = authorizeCmd->getId();
    int nozzle = authorizeCmd->getNozzle();
    int price = authorizeCmd->getPrice();
    int volume = authorizeCmd->getVolume();
    auto productObj = this->mediator_->getProductOnPumpNozzle(pumpId, nozzle);

    auto query = fmt::format(R"(INSERT INTO transactions (enddate, pump, nozzle, fueltype, price, volume, sum, user, requestedvolume, requestedsum)
                    VALUES("", {}, {}, "{}", {}, 0, 0, "{}", {}, 0);)", pumpId, nozzle, productObj.getName(), price, user, volume);

    char *errMsg = nullptr;
    auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);

    LogFile(spdlog::level::trace, "|SQL| Request: {}", query);
    LogFile(spdlog::level::trace, "|SQL| Response: {}", rc);
    if(rc != SQLITE_OK) {
        LogPrintf(spdlog::level::err, "|DB| Cannot add new transaction: {}", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        auto transactionId = static_cast<int>(sqlite3_last_insert_rowid(db));
        authorizeCmd->setDbTransactionId(transactionId);
        authorizeCmd->setDbId(transactionId);
        LogPrintf(spdlog::level::info, "|DB| Inserted transaction |{}| new state: |0|", transactionId);
    }
    return authorizeCmd;
}


/*
 * productid INT NOT NULL
 * "productname TEXT NOT NULL,"
 * "productprice INT NOT NULL,"
 * "productexpanssion REAL NOT NULL)
*/

void TransactionControl::addProduct(const std::shared_ptr<FuelProduct>& fuelUnit) {
    auto query = fmt::format(R"(INSERT INTO prices (productid, productname, productprice, productexpanssion)
                    VALUES({}, '{}', {}, 0) ON CONFLICT(productid) DO UPDATE SET productname = '{}', productprice = {};)",
                    fuelUnit->getId(), fuelUnit->getName(), fuelUnit->getPrice(), fuelUnit->getName(), fuelUnit->getPrice());
    char *errMsg = nullptr;
    auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);

    if(rc != SQLITE_OK) {
        LogPrintf(spdlog::level::err, "|DB| Cannot add new product: {}", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        LogPrintf(spdlog::level::info, "|DB| Inserted product");
    }
}

void TransactionControl::updateProduct(const std::shared_ptr<FuelProduct>& fuelUnit) {
    auto query = fmt::format(R"(UPDATE prices SET productname = '{}', productprice = {} WHERE productid = {};)",
                             fuelUnit->getName(), fuelUnit->getPrice(), fuelUnit->getId());
    char *errMsg = nullptr;
    auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);

    if(rc != SQLITE_OK) {
        LogPrintf(spdlog::level::err, "|DB| Cannot update product: {}", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        LogPrintf(spdlog::level::info, "|DB| Updated product");
    }
}

/*
 * sql = "CREATE TABLE IF NOT EXISTS totals (id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "measure_date TEXT NOT NULL DEFAULT (datetime('now')), "
          "column INT NOT NULL, "
          "nozzle INT NOT NULL, "
          "product_id INT NOT NULL, "
          "volume INT NOT NULL);";
 */

void TransactionControl::addTotalKassaInfo(const std::shared_ptr<UniversalTotals> &data) {
    int pumpId = data->getId();
    int nozzle = data->getNozzle();
    long long volume = data->getVolume();
    auto productObj = this->mediator_->getProductOnPumpNozzle(pumpId, nozzle);
    int productId = productObj.getId();

    auto query = fmt::format(R"(INSERT INTO totals (column, nozzle, product_id, volume)
                    VALUES({}, {}, {}, {});)", pumpId, nozzle, productId, volume);

    char *errMsg = nullptr;
    auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);

    if(rc != SQLITE_OK) {
        LogPrintf(spdlog::level::err, "|DB| Cannot add new total record: {}", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        auto transactionId = static_cast<int>(sqlite3_last_insert_rowid(db));
        LogPrintf(spdlog::level::info, "|DB| Inserted new total record", transactionId);
    }
}

std::shared_ptr<UniversalTotals> TransactionControl::getKassaTotals(std::unique_ptr<UniversalCmd> cmd) {
    sqlite3_stmt* stmt;
    std::string actucalCfg;
    std::shared_ptr<UniversalTotals> totalResponse;
    int pumpId = cmd->getId();

    auto query = fmt::format(R"(SELECT * FROM totals WHERE id = (SELECT MAX(id) FROM totals WHERE pump = {});)", pumpId);
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    if(stmt != nullptr) {
        int volumeTotal = sqlite3_column_int(stmt, 5);
        int sumTotal = sqlite3_column_int(stmt, 6);
        totalResponse->setSum(sumTotal);
        totalResponse->setVolume(volumeTotal);
        totalResponse->setId(pumpId);
        totalResponse->setType(StateType::TOTAL_RESPONSE);
        sqlite3_finalize(stmt);
        return totalResponse;
    } else {
        sqlite3_finalize(stmt);
        return totalResponse;
    }
}

std::shared_ptr<FuelProduct> TransactionControl::getProductInfo(int productId) {
    sqlite3_stmt* stmt;
    std::string actucalCfg;
    std::shared_ptr<FuelProduct> cfgPrice;

    auto query = fmt::format(R"(SELECT * FROM prices WHERE id = {} ORDER BY id DESC LIMIT 1;)", productId);
    sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    sqlite3_step(stmt);
    if(stmt != nullptr) {
        std::string _name = (char *)sqlite3_column_text(stmt, 1);
        int _price = sqlite3_column_int(stmt, 2);
        cfgPrice->setId(productId);
        cfgPrice->setPrice(_price);
        cfgPrice->setName(_name);
        sqlite3_finalize(stmt);
        return cfgPrice;
    } else {
        sqlite3_finalize(stmt);
        return nullptr;
    }
}

std::unique_ptr<UniversalCmd>
TransactionControl::addSimulation(std::unique_ptr<UniversalCmd> data, const std::string &user) {
    auto techCmd = std::make_unique<TechCmd>(*dynamic_cast<TechCmd*>(data.release()));
    int pumpId = techCmd->getId();
    int nozzle = techCmd->getNozzle();
    auto productObj = this->mediator_->getProductOnPumpNozzle(pumpId, nozzle);

    auto query = fmt::format(R"(INSERT INTO simulations (enddate, pump, nozzle, fueltype, price, volume, sum, user, requestedvolume, requestedsum)
                    VALUES("", {}, {}, "{}", {}, 0, 0, "{}", {}, 0);)", pumpId, nozzle, productObj.getName(), 0, user, 0);

    char *errMsg = nullptr;
    auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);

    if(rc != SQLITE_OK) {
        LogPrintf(spdlog::level::err, "|DB| Cannot add new simulations: {}", errMsg);
        sqlite3_free(errMsg);
    }
    else {
        auto transactionId = static_cast<int>(sqlite3_last_insert_rowid(db));
        LogPrintf(spdlog::level::info, "|DB| Inserted simulation |{}| new state: |0|", transactionId);
    }
    return techCmd;
}

std::unique_ptr<UniversalCmd>
TransactionControl::updateSimulationFromCmd(std::unique_ptr<UniversalCmd> data, const std::string &user) {
    switch(data->getCmdType()) {
        case CmdType::STOP_COMMAND: {
            auto query = fmt::format(
                    R"(UPDATE simulations SET state = 3, enddate = '{}', user = '{}' WHERE id = (SELECT MAX(id) FROM simulations WHERE pump = {}) AND state = 2)",
                    getTimeNow().c_str(), user.c_str(), data->getId());
            char *errMsg = nullptr;
            auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK) {
                LogPrintf(spdlog::level::err, "|DB| Cannot update simulation: {}", errMsg);
                sqlite3_free(errMsg);
            } else {
                LogPrintf(spdlog::level::info, "Updated simulation with state |3|");
            }
            break;
        }
        case CmdType::WRITE_VOLUME: {
            auto authorizeCmd = dynamic_unique_cast<UniversalAuthorize>(std::move(data));
            int pumpId = authorizeCmd->getId();
            int nozzle = authorizeCmd->getNozzle();
            int price = authorizeCmd->getPrice();
            int volume = authorizeCmd->getVolume();
            auto productObj = this->mediator_->getProductOnPumpNozzle(pumpId, nozzle);

            auto query = fmt::format(R"(UPDATE simulations SET state = 2, fueltype = '{}', user = '{}', price = {}, requestedvolume = {}
                            WHERE id = (SELECT MAX(id) FROM simulations WHERE pump = {}) AND state = 1;)", productObj.getName(), user.c_str(), price, volume, pumpId);
            char *errMsg = nullptr;
            auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK) {
                LogPrintf(spdlog::level::err, "|DB| Cannot update simulation: {}", errMsg);
                sqlite3_free(errMsg);
            } else {
                LogPrintf(spdlog::level::info, "Updated simulation with state |2|");
            }
            return authorizeCmd;
        }
        default:
            break;
    }
    return data;
}

std::unique_ptr<UniversalCmd> TransactionControl::updateTransactionFromCmd(std::unique_ptr<UniversalCmd> data) {
    switch(data->getCmdType()) {
        case CmdType::CLOSE_REPORT: {
            auto query = fmt::format(
                    R"(UPDATE transactions SET state = 4, enddate = '{}' WHERE id = (SELECT MAX(id) FROM transactions WHERE pump = {}) AND state = 3;)",
                    getTimeNow().c_str(), data->getId());
            char *errMsg = nullptr;
            LogFile(spdlog::level::trace, "|SQL| Last state on transaction: {}", getLastStateOnTransaction(data->getId()));
            auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
            LogFile(spdlog::level::trace, "|SQL| Request: {}", query);
            LogFile(spdlog::level::trace, "|SQL| Response: {}", rc);
            if (rc != SQLITE_OK) {
                LogPrintf(spdlog::level::err, "|DB| Cannot update transaction: {}", errMsg);
                sqlite3_free(errMsg);
            } else if(sqlite3_changes(db) > 0) {
                LogPrintf(spdlog::level::info, "Updated transaction with state |4|");
            }
            break;
        }
        default:
            break;
    }
    return data;
}

void TransactionControl::updateTransaction(const std::shared_ptr<UniversalState>& data) {
    switch(data->getType()) {
        case StateType::SUPPLY_DONE: {
            std::shared_ptr<UniversalAmount> amountData = std::dynamic_pointer_cast<UniversalAmount>(data);
            auto query = fmt::format(R"(UPDATE transactions SET state = 3, enddate = '{}', volume = {}, sum = {}, trktxnid = {} WHERE id = (SELECT MAX(id) FROM transactions WHERE pump = {}) AND state = 1;)",
                                     getTimeNow().c_str() , amountData->getVolume(), amountData->getSum(), amountData->getDbTransactionId(), amountData->getId());
            char *errMsg = nullptr;
            LogFile(spdlog::level::trace, "|SQL| Last state on transaction: {}", getLastStateOnTransaction(data->getId()));
            auto rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
            LogFile(spdlog::level::trace, "|SQL| Request: {}", query);
            LogFile(spdlog::level::trace, "|SQL| Response: {}", rc);
            if(rc != SQLITE_OK) {
                LogPrintf(spdlog::level::err, "|DB| Cannot update transaction: {}", errMsg);
                sqlite3_free(errMsg);
            } else if(sqlite3_changes(db) > 0) {
                LogPrintf(spdlog::level::info, "Updated transaction with state |3|");
            }
        }
        break;
        default:
            break;
    }
}

TransactionControl::TransactionControl() {
    std::string sql;

    openDB();
    initTables();
}
