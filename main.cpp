#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "app/pipeline.h"
#include "infra/nmea_parser.h"
#include "infra/gps_history.h"
#include "infra/json_config.h"
#include "infra/filter_factory.h"
#include "infra/display_factory.h"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <nmea_file>" << std::endl;
    std::cout << "Example: " << programName << " ../data/sample.nmea" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string nmeaFile = argv[1];
    std::string configFile = "../data/config.json";

    // Load configuration
    JsonConfig config;
    if (!config.loadFromFile(configFile)) {
        std::cerr << "Error loading config from " << configFile << std::endl;
        return 1;
    }

    // === Composition Root: wire all dependencies ===
    auto parser  = std::make_unique<NmeaParser>();
    auto history = std::make_unique<GpsHistory>(config.getHistorySize());
    auto display = DisplayFactory::create(config);

    GpsPipeline pipeline(std::move(parser), std::move(history), std::move(display));

    for (const auto& filterConfig : config.getFilters()) {
        auto filter = FilterFactory::create(filterConfig);
        if (filter) {
            pipeline.addFilter(std::move(filter), filterConfig.priority);
        }
    }

    // Open and process file
    std::ifstream file(nmeaFile);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << nmeaFile << std::endl;
        return 1;
    }

    std::cout << "Processing file: " << nmeaFile << std::endl;
    std::cout << "Config: " << configFile << std::endl;

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
