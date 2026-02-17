#pragma once

#include "domain/i_filter.h"
#include <cmath>

class JumpFilter : public IGpsFilter {
public:
    explicit JumpFilter(double maxJumpMeters = 100.0);

    FilterResult process(GpsPoint& point, const IHistory& history) override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    std::string getName() const override;

    void setMaxJump(double maxJump);
    double getMaxJump() const;

private:
    double calculateDistance(const GpsPoint& p1, const GpsPoint& p2) const;

    bool enabled_;
    double maxJumpMeters_;
};
