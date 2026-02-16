#include "pipeline.h"
#include "console_display.h"
#include "file_display.h"
#include "satellite_filter.h"
#include "speed_filter.h"
#include "jump_filter.h"
#include "stop_filter.h"
#include "smoothing_filter.h"
#include <algorithm>
#include <iostream>

// Основной конструктор с конфигурацией
GpsPipeline::GpsPipeline(const JsonConfig& config)
    : config_(config)
    , history_(config.getHistorySize()) {
    
    // Создание дисплея согласно конфигурации
    display_ = createDisplay(config);
    
    // Создание и настройка фильтров согласно конфигурации
    setupFilters(config);
}

// Альтернативный конструктор для тестов
GpsPipeline::GpsPipeline(std::unique_ptr<IDisplay> display)
    : display_(std::move(display))
    , history_(10) {
    // Конфигурация по умолчанию
    config_.setHistorySize(10);
    config_.setDisplayType("console");
}

GpsPipeline::~GpsPipeline() = default;

std::unique_ptr<IDisplay> GpsPipeline::createDisplay(const JsonConfig& config) {
    if (config.getDisplayType() == "file" && !config.getOutputFile().empty()) {
        return std::make_unique<FileDisplay>(
            config.getOutputFile(),
            config.isFileRotation(),
            config.getMaxFileSize()
        );
    }
    return std::make_unique<ConsoleDisplay>();
}

std::unique_ptr<IGpsFilter> GpsPipeline::createFilter(const FilterConfig& config) {
    std::unique_ptr<IGpsFilter> filter;
    
    if (config.type == "SatelliteFilter") {
        int minSatellites = 4;
        auto it = config.params.find("minSatellites");
        if (it != config.params.end()) {
            minSatellites = static_cast<int>(it->second);
        }
        filter = std::make_unique<SatelliteFilter>(minSatellites);
    }
    else if (config.type == "SpeedFilter") {
        double maxSpeed = 300.0;
        auto it = config.params.find("maxSpeed");
        if (it != config.params.end()) {
            maxSpeed = it->second;
        }
        filter = std::make_unique<SpeedFilter>(maxSpeed);
    }
    else if (config.type == "JumpFilter") {
        double maxJump = 100.0;
        auto it = config.params.find("maxJump");
        if (it != config.params.end()) {
            maxJump = it->second;
        }
        filter = std::make_unique<JumpFilter>(maxJump);
    }
    else if (config.type == "StopFilter") {
        double threshold = 3.0;
        auto it = config.params.find("threshold");
        if (it != config.params.end()) {
            threshold = it->second;
        }
        filter = std::make_unique<StopFilter>(threshold);
    }
    else if (config.type == "SmoothingFilter") {
        double cutoff = 0.1;
        double sampleRate = 1.0;
        
        auto it = config.params.find("cutoffFrequency");
        if (it != config.params.end()) {
            cutoff = it->second;
        }
        
        it = config.params.find("sampleRate");
        if (it != config.params.end()) {
            sampleRate = it->second;
        }
        
        filter = std::make_unique<SmoothingFilter>(cutoff, sampleRate);
    }
    else {
        std::cerr << "Warning: Unknown filter type '" << config.type << "'\n";
        return nullptr;
    }
    
    filter->setEnabled(config.enabled);
    return filter;
}

void GpsPipeline::setupFilters(const JsonConfig& config) {
    filters_.clear();
    
    for (const auto& filterConfig : config.getFilters()) {
        auto filter = createFilter(filterConfig);
        if (filter) {
            filters_.emplace_back(filterConfig.priority, std::move(filter));
        }
    }
    
    // Сортировка по приоритету (меньший приоритет = выполняется раньше)
    std::sort(filters_.begin(), filters_.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
}

void GpsPipeline::addFilter(std::unique_ptr<IGpsFilter> filter, int priority) {
    filters_.emplace_back(priority, std::move(filter));
    // Сортировка по приоритету (меньший приоритет = выполняется раньше)
    std::sort(filters_.begin(), filters_.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
}

void GpsPipeline::applyFilters(GpsPoint& point) {
    for (auto& [priority, filter] : filters_) {
        if (!filter->isEnabled()) continue;
        
        FilterResult result = filter->process(point, history_);
        
        if (result == FilterResult::REJECT) {
            rejectedCount_++;
            display_->showRejected(filter->getName() + ": point rejected");
            return;
        }
        else if (result == FilterResult::STOP) {
            // Точка обработана фильтром остановки
            break;
        }
        // PASS - продолжаем
    }
    
    // Точка прошла все фильтры
    validCount_++;
    history_.addPoint(point);
    display_->showPoint(point);
}

void GpsPipeline::process(const std::string& nmeaLine) {
    processedCount_++;
    
    auto pointOpt = parser_.parseLine(nmeaLine);
    
    if (!pointOpt.has_value()) {
        errorCount_++;
        display_->showParseError("invalid checksum or message format");
        return;
    }
    
    GpsPoint point = *pointOpt;
    
    if (!point.isValid) {
        display_->showInvalidFix(point.timestamp);
        return;
    }
    
    applyFilters(point);
}

void GpsPipeline::setHistorySize(size_t size) {
    history_.setMaxSize(size);
}

GpsHistory& GpsPipeline::getHistory() {
    return history_;
}

const GpsHistory& GpsPipeline::getHistory() const {
    return history_;
}

int GpsPipeline::getProcessedCount() const {
    return processedCount_;
}

int GpsPipeline::getValidCount() const {
    return validCount_;
}

int GpsPipeline::getRejectedCount() const {
    return rejectedCount_;
}

int GpsPipeline::getErrorCount() const {
    return errorCount_;
}

const JsonConfig& GpsPipeline::getConfig() const {
    return config_;
}