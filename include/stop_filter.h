#pragma once

#include "filter_interface.h"

class StopFilter : public IGpsFilter {
public:
    explicit StopFilter(double speedThresholdKmh = 3.0);
    
    FilterResult process(GpsPoint& point, const GpsHistory& history) override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    std::string getName() const override;
    
    void setThreshold(double threshold);
    double getThreshold() const;
    
private:
    bool enabled_;
    double speedThresholdKmh_;
};