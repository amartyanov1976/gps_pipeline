#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "parser.h"
#include "history.h"
#include "filter_interface.h"
#include "display_interface.h"
#include "json_config.h"

class GpsPipeline {
public:
    // Конструктор принимает объект JsonConfig и создает все компоненты
    explicit GpsPipeline(const JsonConfig& config);
    
    // Альтернативный конструктор для обратной совместимости (для тестов)
    explicit GpsPipeline(std::unique_ptr<IDisplay> display);
    
    ~GpsPipeline();
    
    // Добавление фильтров (для ручной настройки)
    void addFilter(std::unique_ptr<IGpsFilter> filter, int priority = 0);
    
    // Обработка одной NMEA строки
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
    
    // Получить текущую конфигурацию
    const JsonConfig& getConfig() const;

private:
    // Приватные методы для создания компонентов
    std::unique_ptr<IDisplay> createDisplay(const JsonConfig& config);
    std::unique_ptr<IGpsFilter> createFilter(const FilterConfig& config);
    void setupFilters(const JsonConfig& config);
    void applyFilters(GpsPoint& point);
    
    NmeaParser parser_;
    GpsHistory history_;
    std::unique_ptr<IDisplay> display_;
    JsonConfig config_;
    
    std::vector<std::pair<int, std::unique_ptr<IGpsFilter>>> filters_;
    
    int processedCount_ = 0;
    int validCount_ = 0;
    int rejectedCount_ = 0;
    int errorCount_ = 0;
};