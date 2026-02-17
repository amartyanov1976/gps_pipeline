#include "infra/satellite_filter.h"

SatelliteFilter::SatelliteFilter(int minSatellites)
    : enabled_(true), minSatellites_(minSatellites) {}

FilterResult SatelliteFilter::process(GpsPoint& point, const IHistory& /*history*/) {
    if (!enabled_) return FilterResult::PASS;

    if (!point.isValid) {
        return FilterResult::REJECT;
    }

    if (point.satellites < minSatellites_ && point.satellites > 0) {
        return FilterResult::REJECT;
    }

    return FilterResult::PASS;
}

void SatelliteFilter::setEnabled(bool enabled) { enabled_ = enabled; }
bool SatelliteFilter::isEnabled() const { return enabled_; }
std::string SatelliteFilter::getName() const { return "SatelliteFilter"; }
void SatelliteFilter::setMinSatellites(int min) { minSatellites_ = min; }
int SatelliteFilter::getMinSatellites() const { return minSatellites_; }
