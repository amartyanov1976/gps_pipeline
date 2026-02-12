#pragma once

#include "gps_point.h"
#include "filters/filter.h"
#include <pipeline/pipeline.h>
#include <memory>
#include <vector>
#include <deque>
#include <mutex>

namespace gps {

class GpsPipeline {
public:
    explicit GpsPipeline(size_t historySize = 10);
    ~GpsPipeline();
    
    GpsPipeline(const GpsPipeline&) = delete;
    GpsPipeline& operator=(const GpsPipeline&) = delete;
    GpsPipeline(GpsPipeline&& other) noexcept;
    GpsPipeline& operator=(GpsPipeline&& other) noexcept;
    
    // Управление фильтрами
    void addFilter(std::unique_ptr<Filter> filter);
    void removeFilter(size_t index);
    void clearFilters();
    
    void enableFilter(size_t index, bool enabled);
    Filter* getFilter(size_t index);
    const Filter* getFilter(size_t index) const;
    size_t getFilterCount() const;
    
    // Обработка точки
    FilterResult process(GpsPoint& point);
    
    // Управление историей
    void setHistorySize(size_t size);
    size_t getHistorySize() const;
    const std::deque<GpsPoint>& getHistory() const;
    void clearHistory();
    
    // Статистика
    size_t getProcessedCount() const;
    size_t getRejectedCount() const;
    size_t getStoppedCount() const;
    void resetStats();

private:
    void updateHistory(const GpsPoint& point);
    void cleanup();
    
    pipeline* pipeline_;
    std::vector<std::unique_ptr<Filter>> filters_;
    mutable std::deque<GpsPoint> history_;
    size_t historySize_;
    
    // Статистика
    size_t processedCount_;
    size_t rejectedCount_;
    size_t stoppedCount_;
    
    mutable std::mutex mutex_;
};

} // namespace gps