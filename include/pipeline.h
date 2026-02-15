#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "parser.h"
#include "history.h"
#include "filter_interface.h"
#include "display_interface.h"

class GpsPipeline {
public:
    explicit GpsPipeline(std::unique_ptr<IDisplay> display);
    ~GpsPipeline();
    
    // Добавление фильтров
    void addFilter(std::unique_ptr<IGpsFilter> filter, int priority = 0);
    
    // Обработка одной NMEA строки (основной метод)
    void process(const std::string& nmeaLine);
    
    // Настройка
    void setHistorySize(size_t size);
    GpsHistory& getHistory();
    const GpsHistory& getHistory() const;
    
    // Статистика
    int getProcessedCount() const;
    int getValidCount() const;
    int getRejectedCount() const;
    int getErrorCount() const;
    
private:
    void applyFilters(GpsPoint& point);
    
    NmeaParser parser_;
    GpsHistory history_;
    std::unique_ptr<IDisplay> display_;
    
    std::vector<std::pair<int, std::unique_ptr<IGpsFilter>>> filters_;
    
    int processedCount_ = 0;
    int validCount_ = 0;
    int rejectedCount_ = 0;
    int errorCount_ = 0;
};