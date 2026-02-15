#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "pipeline.h"
#include "console_display.h"
#include "satellite_filter.h"
#include "speed_filter.h"
#include "jump_filter.h"
#include "stop_filter.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <nmea_file>\n";
        return 1;
    }
    
    std::string filename = argv[1];
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }
    
    // Создаём пайплайн с консольным выводом
    auto display = std::make_unique<ConsoleDisplay>();
    GpsPipeline pipeline(std::move(display));
    
    // Настраиваем фильтры
    auto satelliteFilter = std::make_unique<SatelliteFilter>(4);  // минимум 4 спутника
    auto speedFilter = std::make_unique<SpeedFilter>(300.0);      // макс 300 км/ч
    auto jumpFilter = std::make_unique<JumpFilter>(100.0);        // макс скачок 100 м
    auto stopFilter = std::make_unique<StopFilter>(3.0);          // порог остановки 3 км/ч
    
    pipeline.addFilter(std::move(satelliteFilter), 1);   // высокий приоритет
    pipeline.addFilter(std::move(speedFilter), 2);
    pipeline.addFilter(std::move(jumpFilter), 3);
    pipeline.addFilter(std::move(stopFilter), 4);        // низкий приоритет
    
    // Читаем файл построчно и передаём в пайплайн
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Удаляем \r если есть
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        pipeline.process(line);
    }
    
    file.close();
    
    // Выводим статистику
    std::cout << "\n=== Statistics ===\n";
    std::cout << "Processed: " << pipeline.getProcessedCount() << "\n";
    std::cout << "Valid: " << pipeline.getValidCount() << "\n";
    std::cout << "Rejected: " << pipeline.getRejectedCount() << "\n";
    std::cout << "Errors: " << pipeline.getErrorCount() << "\n";
    
    return 0;
}