#pragma once

#include "filter_interface.h"

class SatelliteFilter : public IGpsFilter {
public:
    explicit SatelliteFilter(int minSatellites = 4);
    
    FilterResult process(GpsPoint& point, const GpsHistory& history) override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    std::string getName() const override;
    
    void setMinSatellites(int min);
    int getMinSatellites() const;
    
private:
    bool enabled_;
    int minSatellites_;
};