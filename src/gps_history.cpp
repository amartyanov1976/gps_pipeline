#include "infra/gps_history.h"

GpsHistory::GpsHistory(size_t maxSize) : maxSize_(maxSize) {}

GpsHistory::~GpsHistory() = default;

void GpsHistory::addPoint(const GpsPoint& point) {
    std::lock_guard<std::mutex> lock(mutex_);
    points_.push_back(point);
    if (points_.size() > maxSize_) {
        points_.pop_front();
    }
}

std::optional<GpsPoint> GpsHistory::getLastValid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = points_.rbegin(); it != points_.rend(); ++it) {
        if (it->isValid) {
            return *it;
        }
    }
    return std::nullopt;
}

std::deque<GpsPoint> GpsHistory::getAllPoints() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return points_;
}

void GpsHistory::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    points_.clear();
}

size_t GpsHistory::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return points_.size();
}

bool GpsHistory::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return points_.empty();
}

void GpsHistory::setMaxSize(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxSize_ = maxSize;
    while (points_.size() > maxSize_) {
        points_.pop_front();
    }
}

size_t GpsHistory::getMaxSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return maxSize_;
}
