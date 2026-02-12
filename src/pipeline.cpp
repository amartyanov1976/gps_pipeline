#include "pipeline.h"

namespace gps {

GpsPipeline::GpsPipeline(size_t historySize) 
    : pipeline_(pipeline_new())
    , historySize_(historySize)
    , processedCount_(0)
    , rejectedCount_(0)
    , stoppedCount_(0) {}

GpsPipeline::~GpsPipeline() {
    cleanup();
}

GpsPipeline::GpsPipeline(GpsPipeline&& other) noexcept
    : pipeline_(std::exchange(other.pipeline_, nullptr))
    , filters_(std::move(other.filters_))
    , history_(std::move(other.history_))
    , historySize_(other.historySize_)
    , processedCount_(other.processedCount_)
    , rejectedCount_(other.rejectedCount_)
    , stoppedCount_(other.stoppedCount_) {}

GpsPipeline& GpsPipeline::operator=(GpsPipeline&& other) noexcept {
    if (this != &other) {
        cleanup();
        pipeline_ = std::exchange(other.pipeline_, nullptr);
        filters_ = std::move(other.filters_);
        history_ = std::move(other.history_);
        historySize_ = other.historySize_;
        processedCount_ = other.processedCount_;
        rejectedCount_ = other.rejectedCount_;
        stoppedCount_ = other.stoppedCount_;
    }
    return *this;
}

void GpsPipeline::cleanup() {
    if (pipeline_) {
        pipeline_free(pipeline_);
        pipeline_ = nullptr;
    }
}

void GpsPipeline::addFilter(std::unique_ptr<Filter> filter) {
    std::lock_guard<std::mutex> lock(mutex_);
    pipeline_command_args(pipeline_, "filter", filter->name().c_str(), nullptr);
    filters_.push_back(std::move(filter));
}

void GpsPipeline::removeFilter(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < filters_.size()) {
        filters_.erase(filters_.begin() + index);
    }
}

void GpsPipeline::clearFilters() {
    std::lock_guard<std::mutex> lock(mutex_);
    filters_.clear();
}

void GpsPipeline::enableFilter(size_t index, bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < filters_.size()) {
        filters_[index]->setEnabled(enabled);
    }
}

Filter* GpsPipeline::getFilter(size_t index) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < filters_.size()) {
        return filters_[index].get();
    }
    return nullptr;
}

const Filter* GpsPipeline::getFilter(size_t index) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < filters_.size()) {
        return filters_[index].get();
    }
    return nullptr;
}

size_t GpsPipeline::getFilterCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return filters_.size();
}

FilterResult GpsPipeline::process(GpsPoint& point) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    processedCount_++;
    
    if (!point.isValid()) {
        rejectedCount_++;
        return FilterResult::REJECT;
    }
    
    for (auto& filter : filters_) {
        if (!filter->isEnabled()) continue;
        
        auto result = filter->process(point, history_);
        
        switch (result) {
            case FilterResult::REJECT:
                rejectedCount_++;
                return FilterResult::REJECT;
            case FilterResult::STOP:
                stoppedCount_++;
                updateHistory(point);
                return FilterResult::STOP;
            case FilterResult::PASS:
                continue;
        }
    }
    
    updateHistory(point);
    return FilterResult::PASS;
}

void GpsPipeline::updateHistory(const GpsPoint& point) {
    history_.push_back(point);
    while (history_.size() > historySize_) {
        history_.pop_front();
    }
}

void GpsPipeline::setHistorySize(size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    historySize_ = size;
    
    // Усекаем историю при необходимости
    while (history_.size() > historySize_) {
        history_.pop_front();
    }
}

size_t GpsPipeline::getHistorySize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return historySize_;
}

const std::deque<GpsPoint>& GpsPipeline::getHistory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return history_;
}

void GpsPipeline::clearHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    history_.clear();
}

size_t GpsPipeline::getProcessedCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return processedCount_;
}

size_t GpsPipeline::getRejectedCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return rejectedCount_;
}

size_t GpsPipeline::getStoppedCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stoppedCount_;
}

void GpsPipeline::resetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    processedCount_ = 0;
    rejectedCount_ = 0;
    stoppedCount_ = 0;
}

} // namespace gps