#pragma once

#include <string>
#include <cmath>

struct GpsPoint {
    double latitude = 0.0;      // градусы, DD.DDDDD
    double longitude = 0.0;     // градусы, DD.DDDDD
    double speed = 0.0;         // км/ч
    double course = 0.0;        // градусы, 0-360
    double altitude = 0.0;      // метры
    int satellites = 0;
    float hdop = 0.0f;
    unsigned long long timestamp = 0;  // миллисекунды UTC
    bool isValid = false;
    
    std::string toString() const;
    bool operator==(const GpsPoint& other) const;
};