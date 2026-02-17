#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "domain/i_parser.h"
#include "domain/i_history.h"
#include "domain/i_filter.h"
#include "domain/i_display.h"

class GpsPipeline {
public:
    GpsPipeline(
        std::unique_ptr<IParser> parser,
        std::unique_ptr<IHistory> history,
        std::unique_ptr<IDisplay> display
    );
    ~GpsPipeline();

    void addFilter(std::unique_ptr<IGpsFilter> filter, int priority = 0);
    void process(const std::string& line);

    void setHistorySize(size_t size);
    IHistory& getHistory();
    const IHistory& getHistory() const;

    int getProcessedCount() const;
    int getValidCount() const;
    int getRejectedCount() const;
    int getErrorCount() const;

private:
    void applyFilters(GpsPoint& point);

    std::unique_ptr<IParser> parser_;
    std::unique_ptr<IHistory> history_;
    std::unique_ptr<IDisplay> display_;

    std::vector<std::pair<int, std::unique_ptr<IGpsFilter>>> filters_;

    int processedCount_ = 0;
    int validCount_ = 0;
    int rejectedCount_ = 0;
    int errorCount_ = 0;
};
