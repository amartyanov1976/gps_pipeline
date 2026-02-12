#include "nmea_parser.h"
#include "pipeline.h"
#include "display.h"
#include "filters/satellite_filter.h"
#include "filters/speed_filter.h"
#include "filters/jump_filter.h"
#include "filters/stop_filter.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <iomanip>

void printUsage(const char* program) {
    std::cout << "GPS Data Processing Pipeline with libpipeline\n"
              << "==============================================\n"
              << "Usage: " << program << " <nmea_file>\n"
              << "       " << program << " --help\n\n"
              << "Options:\n"
              << "  <nmea_file>    Path to NMEA file\n"
              << "  --help         Show this help\n\n"
              << "Example:\n"
              << "  " << program << " ../data/sample.nmea\n";
}

void printConfig(const gps::GpsPipeline& pipeline) {
    std::cout << "\nPipeline Configuration:\n"
              << "  History size: " << pipeline.getHistorySize() << " points\n"
              << "  Active filters: " << pipeline.getFilterCount() << "\n";
    
    for (size_t i = 0; i < pipeline.getFilterCount(); ++i) {
        auto* filter = pipeline.getFilter(i);
        if (filter) {
            std::cout << "    " << i+1 << ". " << filter->name() 
                     << (filter->isEnabled() ? " (enabled)" : " (disabled)") << "\n";
        }
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string arg = argv[1];
    if (arg == "--help" || arg == "-h") {
        printUsage(argv[0]);
        return 0;
    }
    
    std::string filename = arg;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'\n";
        return 1;
    }
    
    std::cout << "\n🚀 GPS Pipeline with libpipeline\n"
              << "================================\n"
              << "📁 Input: " << filename << "\n";
    
    // Инициализация компонентов
    nmea::Parser parser;
    gps::GpsPipeline pipeline(10);
    gps::ConsoleDisplay display;
    
    // Добавляем фильтры
    pipeline.addFilter(std::make_unique<gps::SatelliteFilter>(4));
    pipeline.addFilter(std::make_unique<gps::SpeedFilter>(200.0));
    pipeline.addFilter(std::make_unique<gps::JumpFilter>(100.0));
    pipeline.addFilter(std::make_unique<gps::StopFilter>(3.0, 5));
    
    printConfig(pipeline);
    
    // Статистика
    int totalPoints = 0;
    int validPoints = 0;
    int rejectedPoints = 0;
    int errorPoints = 0;
    int satRejects = 0;
    int speedRejects = 0;
    int jumpRejects = 0;
    int stopEvents = 0;
    
    // Callback ошибок
    parser.setErrorCallback([&](const std::string& err) {
        display.error("Parse error: " + err);
        errorPoints++;
    });
    
    // Обработка потока
    auto startTime = std::chrono::high_resolution_clock::now();
    
    parser.parseStream(file,
        [&](const GpsPoint& point) {
            totalPoints++;
            GpsPoint processed = point;
            auto result = pipeline.process(processed);
            
            switch (result) {
                case gps::FilterResult::PASS:
                    validPoints++;
                    display.show(processed);
                    break;
                    
                case gps::FilterResult::STOP:
                    validPoints++;
                    stopEvents++;
                    display.show(processed);
                    break;
                    
                case gps::FilterResult::REJECT:
                    rejectedPoints++;
                    if (point.getSatellites() < 4) {
                        satRejects++;
                        display.error("❌ Point rejected: insufficient satellites (" + 
                                    std::to_string(point.getSatellites()) + " < 4)");
                    } else if (point.getSpeed() > 200.0) {
                        speedRejects++;
                        display.error("❌ Point rejected: excessive speed (" + 
                                    std::to_string(static_cast<int>(point.getSpeed())) + " km/h)");
                    } else {
                        jumpRejects++;
                        display.error("❌ Point rejected: coordinate jump detected");
                    }
                    break;
            }
            
            // Небольшая задержка для читаемости
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        },
        [&](const std::string& err) {
            display.error(err);
            errorPoints++;
        }
    );
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
    
    // Итоговая статистика
    std::cout << "\n" << std::string(50, '=') << "\n"
              << "📊 Processing Complete\n"
              << std::string(50, '=') << "\n"
              << "⏱️  Processing time: " << duration.count() << " ms\n"
              << "📈 Throughput: " 
              << (totalPoints * 1000.0 / duration.count()) << " points/sec\n\n"
              
              << "📌 Statistics:\n"
              << "  ├─ Total points:  " << std::setw(6) << totalPoints << "\n"
              << "  ├─ Valid points:  " << std::setw(6) << validPoints 
              << " (" << (totalPoints > 0 ? (validPoints * 100 / totalPoints) : 0) << "%)\n"
              << "  ├─ Rejected:      " << std::setw(6) << rejectedPoints 
              << " (" << (totalPoints > 0 ? (rejectedPoints * 100 / totalPoints) : 0) << "%)\n"
              << "  └─ Errors:        " << std::setw(6) << errorPoints << "\n\n"
              
              << "🔍 Reject reasons:\n"
              << "  ├─ Insufficient satellites: " << std::setw(6) << satRejects << "\n"
              << "  ├─ Excessive speed:        " << std::setw(6) << speedRejects << "\n"
              << "  ├─ Coordinate jumps:       " << std::setw(6) << jumpRejects << "\n"
              << "  └─ Stop events:            " << std::setw(6) << stopEvents << "\n\n"
              
              << "⚙️  Pipeline state:\n"
              << "  ├─ Active filters: " << pipeline.getFilterCount() << "\n"
              << "  ├─ History size:   " << pipeline.getHistory().size() << "/" 
              << pipeline.getHistorySize() << " points\n"
              << "  ├─ Processed:      " << pipeline.getProcessedCount() << "\n"
              << "  ├─ Rejected:       " << pipeline.getRejectedCount() << "\n"
              << "  └─ Stopped:        " << pipeline.getStoppedCount() << "\n"
              << std::string(50, '=') << "\n";
    
    return 0;
}