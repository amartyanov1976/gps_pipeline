#pragma once

#include <deque>
#include <optional>
#include <mutex>
#include "domain/i_history.h"
#include "domain/gps_point.h"

class GpsHistory : public IHistory {
public:
    explicit GpsHistory(size_t maxSize = 10);
    ~GpsHistory() override;

    void addPoint(const GpsPoint& point) override;
    std::optional<GpsPoint> getLastValid() const override;
    std::deque<GpsPoint> getAllPoints() const override;
    void clear() override;
    size_t size() const override;
    bool empty() const override;
    void setMaxSize(size_t maxSize) override;
    size_t getMaxSize() const override;

private:
    size_t maxSize_;
    std::deque<GpsPoint> points_;
    mutable std::mutex mutex_;
};
