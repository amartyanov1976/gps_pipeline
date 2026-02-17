#pragma once

#include "domain/i_filter.h"
#include <deque>
#include <cmath>

class SmoothingFilter : public IGpsFilter {
public:
    explicit SmoothingFilter(double cutoffFrequency = 0.1, double sampleRate = 1.0);

    FilterResult process(GpsPoint& point, const IHistory& history) override;
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
    double cutoffFrequency_;
    double sampleRate_;

    std::deque<GpsPoint> previousPoints_;
    static constexpr size_t MAX_PREVIOUS = 5;
};
