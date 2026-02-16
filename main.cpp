#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "pipeline.h"
#include "json_config.h"

void printUsage(const char* programName) {
    std::cout << "Использование: " << programName << " <nmea_file>" << std::endl;
    std::cout << "Пример: " << programName << " ../data/sample.nmea" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string nmeaFile = argv[1];
    std::string configFile = "../data/config.json";

    // Загрузка конфигурации
    JsonConfig config;
    if (!config.loadFromFile(configFile)) {
        std::cerr << "Ошибка загрузки конфигурации из " << configFile << std::endl;
        return 1;
    }

    // Создание пайплайна с конфигурацией
    GpsPipeline pipeline(config);

    // Открытие файла
    std::ifstream file(nmeaFile);
    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла: " << nmeaFile << std::endl;
        return 1;
    }

    std::cout << "Обработка файла: " << nmeaFile << std::endl;
    std::cout << "Конфигурация: " << configFile << std::endl;

    // Чтение файла и обработка строк
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        pipeline.process(line);
    }

    return 0;
}