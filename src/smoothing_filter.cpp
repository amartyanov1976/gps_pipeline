#include "smoothing_filter.h"
#include <algorithm>

SmoothingFilter::SmoothingFilter(double cutoffFrequency, double sampleRate)
    : enabled_(true)
    , cutoffFrequency_(cutoffFrequency)
    , sampleRate_(sampleRate) {}

double SmoothingFilter::calculateAlpha() const {
    // RC = 1 / (2 * pi * fc)
    // alpha = dt / (RC + dt)
    double dt = 1.0 / sampleRate_;
    double rc = 1.0 / (2.0 * M_PI * cutoffFrequency_);
    return dt / (rc + dt);
}

double SmoothingFilter::lowPassFilter(double current, double previous, double alpha) {
    return previous + alpha * (current - previous);
}

FilterResult SmoothingFilter::process(GpsPoint& point, const GpsHistory& history) {
    if (!enabled_ || !point.isValid) {
        return FilterResult::PASS;
    }
    
    double alpha = calculateAlpha();
    
    // Сохраняем предыдущие точки для сглаживания
    previousPoints_.push_back(point);
    if (previousPoints_.size() > MAX_PREVIOUS) {
        previousPoints_.pop_front();
    }
    
    if (previousPoints_.size() < 2) {
        return FilterResult::PASS; // Недостаточно данных для сглаживания
    }
    
    // Сглаживаем координаты
    const GpsPoint& prev = previousPoints_[previousPoints_.size() - 2];
    
    point.latitude = lowPassFilter(point.latitude, prev.latitude, alpha);
    point.longitude = lowPassFilter(point.longitude, prev.longitude, alpha);
    
    if (point.speed > 0) {
        point.speed = lowPassFilter(point.speed, prev.speed, alpha);
    }
    
    if (point.altitude > 0) {
        point.altitude = lowPassFilter(point.altitude, prev.altitude, alpha);
    }
    
    return FilterResult::PASS;
}

void SmoothingFilter::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool SmoothingFilter::isEnabled() const {
    return enabled_;
}

std::string SmoothingFilter::getName() const {
    return "SmoothingFilter";
}

void SmoothingFilter::setCutoffFrequency(double freq) {
    cutoffFrequency_ = freq;
}

double SmoothingFilter::getCutoffFrequency() const {
    return cutoffFrequency_;
}

void SmoothingFilter::setSampleRate(double rate) {
    sampleRate_ = rate;
}

double SmoothingFilter::getSampleRate() const {
    return sampleRate_;
}