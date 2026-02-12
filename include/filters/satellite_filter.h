#pragma once

#include "filters/filter.h"

namespace gps {

class SatelliteFilter : public Filter {
public:
    explicit SatelliteFilter(int minSatellites = 4);
    
    FilterResult process(GpsPoint& point, const std::deque<GpsPoint>& history) override;
    std::string name() const override;
    
    int getMinSatellites() const;
    void setMinSatellites(int minSatellites);

private:
    int minSatellites_;
};

} // namespace gps