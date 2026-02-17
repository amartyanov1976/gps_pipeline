#pragma once

#include <memory>
#include <string>
#include "domain/gps_point.h"
#include "domain/filter_result.h"
#include "domain/i_history.h"

class IGpsFilter {
public:
    virtual ~IGpsFilter() = default;

    virtual FilterResult process(GpsPoint& point, const IHistory& history) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;
    virtual std::string getName() const = 0;
};

using FilterPtr = std::unique_ptr<IGpsFilter>;
