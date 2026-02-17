#pragma once

#include <string>
#include <cmath>

struct GpsPoint {
    double latitude = 0.0;
    double longitude = 0.0;
    double speed = 0.0;
    double course = 0.0;
    double altitude = 0.0;
    int satellites = 0;
    float hdop = 0.0f;
    unsigned long long timestamp = 0;
    bool isValid = false;

    std::string toString() const;
    bool operator==(const GpsPoint& other) const;
};
