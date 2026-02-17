#include "infra/stop_filter.h"

StopFilter::StopFilter(double speedThresholdKmh)
    : enabled_(true), speedThresholdKmh_(speedThresholdKmh) {}

FilterResult StopFilter::process(GpsPoint& point, const IHistory& history) {
    if (!enabled_) return FilterResult::PASS;

    if (!point.isValid) {
        return FilterResult::REJECT;
    }

    if (point.speed < speedThresholdKmh_) {
        auto lastValid = history.getLastValid();
        if (lastValid.has_value()) {
            point.latitude = lastValid->latitude;
            point.longitude = lastValid->longitude;
            point.speed = 0.0;
        }
        return FilterResult::STOP;
    }

    return FilterResult::PASS;
}

void StopFilter::setEnabled(bool enabled) { enabled_ = enabled; }
bool StopFilter::isEnabled() const { return enabled_; }
std::string StopFilter::getName() const { return "StopFilter"; }
void StopFilter::setThreshold(double threshold) { speedThresholdKmh_ = threshold; }
double StopFilter::getThreshold() const { return speedThresholdKmh_; }
