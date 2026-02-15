#include "speed_filter.h"

SpeedFilter::SpeedFilter(double maxSpeedKmh) 
    : enabled_(true), maxSpeedKmh_(maxSpeedKmh) {}

FilterResult SpeedFilter::process(GpsPoint& point, const GpsHistory& /*history*/) {
    if (!enabled_) return FilterResult::PASS;
    
    if (!point.isValid) {
        return FilterResult::REJECT;
    }
    
    if (point.speed > maxSpeedKmh_ || point.speed < 0) {
        return FilterResult::REJECT;
    }
    
    return FilterResult::PASS;
}

void SpeedFilter::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool SpeedFilter::isEnabled() const {
    return enabled_;
}

std::string SpeedFilter::getName() const {
    return "SpeedFilter";
}

void SpeedFilter::setMaxSpeed(double maxSpeed) {
    maxSpeedKmh_ = maxSpeed;
}

double SpeedFilter::getMaxSpeed() const {
    return maxSpeedKmh_;
}