#pragma once

#include "filter_interface.h"

class SpeedFilter : public IGpsFilter {
public:
    explicit SpeedFilter(double maxSpeedKmh = 300.0);
    
    FilterResult process(GpsPoint& point, const GpsHistory& history) override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    std::string getName() const override;
    
    void setMaxSpeed(double maxSpeed);
    double getMaxSpeed() const;
    
private:
    bool enabled_;
    double maxSpeedKmh_;
};