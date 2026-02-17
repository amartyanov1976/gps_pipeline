#pragma once

#include <deque>
#include <optional>
#include "domain/gps_point.h"

class IHistory {
public:
    virtual ~IHistory() = default;

    virtual void addPoint(const GpsPoint& point) = 0;
    virtual std::optional<GpsPoint> getLastValid() const = 0;
    virtual std::deque<GpsPoint> getAllPoints() const = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
    virtual void setMaxSize(size_t maxSize) = 0;
    virtual size_t getMaxSize() const = 0;
};
