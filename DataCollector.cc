#include "DataCollector.h"

DataCollector& DataCollector::getInstance() {
    static DataCollector instance;
    return instance;
}

void DataCollector::putData(std::vector<std::string> data) {
    database.push_back(data);
}

std::vector<std::vector<std::string>>& DataCollector::getData() {
    return database;
}
