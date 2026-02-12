#pragma once

#include "filters/filter.h"

namespace gps {

class SpeedFilter : public Filter {
public:
    explicit SpeedFilter(double maxSpeed = 200.0);
    
    FilterResult process(GpsPoint& point, const std::deque<GpsPoint>& history) override;
    std::string name() const override;
    
    double getMaxSpeed() const;
    void setMaxSpeed(double maxSpeed);

private:
    double maxSpeed_; // км/ч
};

} // namespace gps