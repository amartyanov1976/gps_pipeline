#pragma once

#include <string>
#include "gps_point.h"

class IDisplay {
public:
    virtual ~IDisplay() = default;
    
    virtual void showPoint(const GpsPoint& point) = 0;
    virtual void showInvalidFix(unsigned long long timestamp) = 0;
    virtual void showParseError(const std::string& error) = 0;
    virtual void showRejected(const std::string& reason) = 0;
    virtual void clear() = 0;
};