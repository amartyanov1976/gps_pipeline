#include "filters/speed_filter.h"

namespace gps {

SpeedFilter::SpeedFilter(double maxSpeed) 
    : maxSpeed_(maxSpeed) {}

FilterResult SpeedFilter::process(GpsPoint& point, const std::deque<GpsPoint>&) {
    if (!enabled_ || !point.isValid()) {
        return FilterResult::PASS;
    }
    
    if (point.getSpeed() > maxSpeed_) {
        return FilterResult::REJECT;
    }
    
    return FilterResult::PASS;
}

std::string SpeedFilter::name() const {
    return "SpeedFilter";
}

double SpeedFilter::getMaxSpeed() const {
    return maxSpeed_;
}

void SpeedFilter::setMaxSpeed(double maxSpeed) {
    maxSpeed_ = maxSpeed;
}

} // namespace gps