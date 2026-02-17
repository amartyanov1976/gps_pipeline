#include "app/pipeline.h"
#include <algorithm>

GpsPipeline::GpsPipeline(
    std::unique_ptr<IParser> parser,
    std::unique_ptr<IHistory> history,
    std::unique_ptr<IDisplay> display)
    : parser_(std::move(parser))
    , history_(std::move(history))
    , display_(std::move(display)) {
}

GpsPipeline::~GpsPipeline() = default;

void GpsPipeline::addFilter(std::unique_ptr<IGpsFilter> filter, int priority) {
    filters_.emplace_back(priority, std::move(filter));
    std::sort(filters_.begin(), filters_.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
}

void GpsPipeline::applyFilters(GpsPoint& point) {
    for (auto& [priority, filter] : filters_) {
        if (!filter->isEnabled()) continue;

        FilterResult result = filter->process(point, *history_);

        if (result == FilterResult::REJECT) {
            rejectedCount_++;
            display_->showRejected(filter->getName() + ": point rejected");
            return;
        }
        else if (result == FilterResult::STOP) {
            break;
        }
    }

    validCount_++;
    history_->addPoint(point);
    display_->showPoint(point);
}

void GpsPipeline::process(const std::string& line) {
    processedCount_++;

    auto pointOpt = parser_->parseLine(line);

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
    history_->setMaxSize(size);
}

IHistory& GpsPipeline::getHistory() {
    return *history_;
}

const IHistory& GpsPipeline::getHistory() const {
    return *history_;
}

int GpsPipeline::getProcessedCount() const { return processedCount_; }
int GpsPipeline::getValidCount() const { return validCount_; }
int GpsPipeline::getRejectedCount() const { return rejectedCount_; }
int GpsPipeline::getErrorCount() const { return errorCount_; }
