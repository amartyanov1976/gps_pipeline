#include "gps_point.h"
#include <cmath>
#include <cstdio>

std::string GpsPoint::toString() const {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), 
        "GpsPoint{lat=%.5f, lon=%.5f, speed=%.1f, course=%.1f, alt=%.1f, sats=%d, hdop=%.1f, valid=%d}",
        latitude, longitude, speed, course, altitude, satellites, hdop, isValid);
    return std::string(buffer);
}

bool GpsPoint::operator==(const GpsPoint& other) const {
    const double eps = 1e-5;
    return std::fabs(latitude - other.latitude) < eps &&
           std::fabs(longitude - other.longitude) < eps &&
           std::fabs(speed - other.speed) < eps &&
           std::fabs(course - other.course) < eps &&
           std::fabs(altitude - other.altitude) < eps &&
           satellites == other.satellites &&
           std::fabs(hdop - other.hdop) < 0.1f &&
           timestamp == other.timestamp &&
           isValid == other.isValid;
}