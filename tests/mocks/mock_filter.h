#pragma once

#include "domain/i_filter.h"
#include <string>

class MockFilter : public IGpsFilter {
public:
    explicit MockFilter(
        const std::string& name = "MockFilter",
        FilterResult result = FilterResult::PASS)
        : name_(name), result_(result), enabled_(true) {}

    FilterResult process(GpsPoint& point, const IHistory& history) override {
        processCalls_++;
        lastPoint_ = point;
        return enabled_ ? result_ : FilterResult::PASS;
    }

    void setEnabled(bool enabled) override { enabled_ = enabled; }
    bool isEnabled() const override { return enabled_; }
    std::string getName() const override { return name_; }

    void setResult(FilterResult result) { result_ = result; }
    int getProcessCalls() const { return processCalls_; }
    const GpsPoint& getLastPoint() const { return lastPoint_; }

private:
    std::string name_;
    FilterResult result_;
    bool enabled_;
    int processCalls_ = 0;
    GpsPoint lastPoint_;
};
