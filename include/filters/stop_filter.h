#pragma once

#include "filters/filter.h"
#include <chrono>

namespace gps {

class StopFilter : public Filter {
public:
    StopFilter(double threshold = 3.0, int minStopTime = 5);
    
    FilterResult process(GpsPoint& point, const std::deque<GpsPoint>& history) override;
    std::string name() const override;
    void reset() override;
    
    double getThreshold() const;
    void setThreshold(double threshold);
    
    int getMinStopTime() const;
    void setMinStopTime(int minStopTime);
    
    bool isStopped() const;

private:
    double threshold_;      // км/ч
    int minStopTime_;       // секунды
    bool stopped_;
    std::chrono::system_clock::time_point stopStartTime_;
    GpsPoint stopPosition_;
};

} // namespace gps