#pragma once

#include <memory>
#include "gps_point.h"
#include "history.h"

enum class FilterResult {
    PASS,      // точка валидна, передать дальше
    REJECT,    // точка невалидна, отбросить
    STOP       // точка обработана, прекратить цепочку
};

class IGpsFilter {
public:
    virtual ~IGpsFilter() = default;
    
    virtual FilterResult process(GpsPoint& point, const GpsHistory& history) = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;
    virtual std::string getName() const = 0;
};

using FilterPtr = std::unique_ptr<IGpsFilter>;