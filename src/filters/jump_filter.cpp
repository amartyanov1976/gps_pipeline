#include "filters/jump_filter.h"
#include <chrono>

namespace gps {

JumpFilter::JumpFilter(double maxJump) 
    : maxJump_(maxJump) {}

FilterResult JumpFilter::process(GpsPoint& point, const std::deque<GpsPoint>& history) {
    if (!enabled_ || !point.isValid() || history.empty()) {
        return FilterResult::PASS;
    }
    
    const auto& last = history.back();
    if (!last.isValid() || !last.hasPosition()) {
        return FilterResult::PASS;
    }
    
    double distance = point.distanceTo(last);
    auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(
        point.getTimestamp() - last.getTimestamp()).count();
    
    if (timeDiff <= 0) {
        return FilterResult::PASS;
    }
    
    double speed = distance / timeDiff;
    if (speed > maxJump_) {
        return FilterResult::REJECT;
    }
    
    return FilterResult::PASS;
}

std::string JumpFilter::name() const {
    return "JumpFilter";
}

double JumpFilter::getMaxJump() const {
    return maxJump_;
}

void JumpFilter::setMaxJump(double maxJump) {
    maxJump_ = maxJump;
}

} // namespace gps