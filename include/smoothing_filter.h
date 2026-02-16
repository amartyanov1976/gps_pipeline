#pragma once

#include "filter_interface.h"
#include <deque>
#include <cmath>

class SmoothingFilter : public IGpsFilter {
public:
    explicit SmoothingFilter(double cutoffFrequency = 0.1, double sampleRate = 1.0);
    
    FilterResult process(GpsPoint& point, const GpsHistory& history) override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    std::string getName() const override;
    
    void setCutoffFrequency(double freq);
    double getCutoffFrequency() const;
    void setSampleRate(double rate);
    double getSampleRate() const;
    
private:
    double lowPassFilter(double current, double previous, double alpha);
    double calculateAlpha() const;
    
    bool enabled_;
    double cutoffFrequency_;  // частота среза в Гц
    double sampleRate_;       // частота дискретизации в Гц
    
    // Кеш для предыдущих значений
    std::deque<GpsPoint> previousPoints_;
    static constexpr size_t MAX_PREVIOUS = 5;
};