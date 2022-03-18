#include "DB.h"
#include "logger.h"

void DB::openDB() {
    int rc;
	bool directoryFound = false;
	const std::string dbDirName("DB");

	if (!directoryFound) {
		LogPrintf(spdlog::level::warn, "|DB| Directory |\"DB\"| do not exists. Creating new one");
//        bool createDir = std::filesystem::create_directory(dbDirName);

//		if (!createDir) {
//			LogPrintf(spdlog::level::err, "|PTS| Cannot create directory |\"DB\"|");
//			exit(1);
//		} else {
//			LogPrintf(spdlog::level::info, "|PTS| Directory created successfully");
//		}
	}

	rc = sqlite3_open("./data.sqlite", &db);
	if (rc) {
		LogPrintf(spdlog::level::err, "|DB| Cannot open database");
		if (sqlite3_close(db)) {
			LogPrintf(spdlog::level::err, "|DB| Cannot close db file");
		}
		exit(1);
	}
}

void DB::initTables() {

}

void DB::addConfig() {

}