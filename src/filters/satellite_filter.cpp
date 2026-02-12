#include "filters/satellite_filter.h"

namespace gps {

SatelliteFilter::SatelliteFilter(int minSatellites) 
    : minSatellites_(minSatellites) {}

FilterResult SatelliteFilter::process(GpsPoint& point, const std::deque<GpsPoint>&) {
    if (!enabled_ || !point.isValid()) {
        return FilterResult::PASS;
    }
    
    if (point.getSatellites() < minSatellites_) {
        return FilterResult::REJECT;
    }
    
    return FilterResult::PASS;
}

std::string SatelliteFilter::name() const {
    return "SatelliteFilter";
}

int SatelliteFilter::getMinSatellites() const {
    return minSatellites_;
}

void SatelliteFilter::setMinSatellites(int minSatellites) {
    minSatellites_ = minSatellites;
}

} // namespace gps