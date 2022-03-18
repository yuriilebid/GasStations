#ifndef PTS2_0_DB_H
#define PTS2_0_DB_H

#include "Mediator.h"
#include <sqlite3.h>

class DB : public BaseComponent{
private:
    sqlite3* db;
public:
    void openDB();
    void initTables();
    void addConfig();
};


#endif //PTS2_0_DB_H
