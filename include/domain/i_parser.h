#pragma once

#include <string>
#include <optional>
#include "domain/gps_point.h"

class IParser {
public:
    virtual ~IParser() = default;

    virtual std::optional<GpsPoint> parseLine(const std::string& line) = 0;
    virtual void reset() = 0;
};
