#pragma once

#include "domain/i_history.h"
#include <deque>
#include <optional>

class MockHistory : public IHistory {
public:
    explicit MockHistory(size_t maxSize = 10) : maxSize_(maxSize) {}

    void addPoint(const GpsPoint& point) override {
        points_.push_back(point);
        if (points_.size() > maxSize_) points_.pop_front();
    }

    std::optional<GpsPoint> getLastValid() const override {
        for (auto it = points_.rbegin(); it != points_.rend(); ++it) {
            if (it->isValid) return *it;
        }
        return std::nullopt;
    }

    std::deque<GpsPoint> getAllPoints() const override { return points_; }
    void clear() override { points_.clear(); }
    size_t size() const override { return points_.size(); }
    bool empty() const override { return points_.empty(); }
    void setMaxSize(size_t maxSize) override { maxSize_ = maxSize; }
    size_t getMaxSize() const override { return maxSize_; }

private:
    size_t maxSize_;
    std::deque<GpsPoint> points_;
};
