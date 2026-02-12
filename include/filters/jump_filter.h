#pragma once

#include "filters/filter.h"

namespace gps {

class JumpFilter : public Filter {
public:
    explicit JumpFilter(double maxJump = 100.0);
    
    FilterResult process(GpsPoint& point, const std::deque<GpsPoint>& history) override;
    std::string name() const override;
    
    double getMaxJump() const;
    void setMaxJump(double maxJump);

private:
    double maxJump_; // м/с
};

} // namespace gps