#include "filters/stop_filter.h"
#include <chrono>

namespace gps {

StopFilter::StopFilter(double threshold, int minStopTime)
    : threshold_(threshold)
    , minStopTime_(minStopTime)
    , stopped_(false)
    , stopStartTime_(std::chrono::system_clock::now())
    , stopPosition_() {}

FilterResult StopFilter::process(GpsPoint& point, const std::deque<GpsPoint>&) {
    if (!enabled_ || !point.isValid() || !point.hasPosition()) {
        return FilterResult::PASS;
    }
    
    if (point.getSpeed() < threshold_) {
        if (!stopped_) {
            // Начинаем остановку
            stopped_ = true;
            stopStartTime_ = point.getTimestamp();
            stopPosition_ = point;
            return FilterResult::PASS;
        } else {
            // Продолжаем остановку
            auto stopDuration = std::chrono::duration_cast<std::chrono::seconds>(
                point.getTimestamp() - stopStartTime_).count();
            
            if (stopDuration >= minStopTime_) {
                // Фиксируем позицию остановки
                point.setLatitude(stopPosition_.getLatitude());
                point.setLongitude(stopPosition_.getLongitude());
                point.setSpeed(0.0);
                return FilterResult::STOP;
            }
        }
    } else {
        // Движение возобновилось
        stopped_ = false;
    }
    
    return FilterResult::PASS;
}

std::string StopFilter::name() const {
    return "StopFilter";
}

void StopFilter::reset() {
    stopped_ = false;
    stopPosition_ = GpsPoint();
}

double StopFilter::getThreshold() const {
    return threshold_;
}

void StopFilter::setThreshold(double threshold) {
    threshold_ = threshold;
}

int StopFilter::getMinStopTime() const {
    return minStopTime_;
}

void StopFilter::setMinStopTime(int minStopTime) {
    minStopTime_ = minStopTime;
}

bool StopFilter::isStopped() const {
    return stopped_;
}

} // namespace gps