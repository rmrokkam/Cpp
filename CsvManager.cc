#include "CsvManager.h"

#include <fstream>

CsvManager& CsvManager::getInstance() {
    static CsvManager instance;
    return instance;
}

void CsvManager::saveToCsv(std::vector<std::vector<std::string>>& db,
                           std::string filename) {
    std::ofstream csvFile(filename);

    if (csvFile.is_open()) {
        for (std::vector<std::string> dataPoint : db) {
            for (std::string component : dataPoint) {
                csvFile << component << ",";
            }
        }
        csvFile.close();
        std::cout << "Data saved to " << filename << std::endl;
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}
