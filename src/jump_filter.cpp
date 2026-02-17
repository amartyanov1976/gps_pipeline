#include "infra/jump_filter.h"
#include <cmath>

JumpFilter::JumpFilter(double maxJumpMeters)
    : enabled_(true), maxJumpMeters_(maxJumpMeters) {}

double JumpFilter::calculateDistance(const GpsPoint& p1, const GpsPoint& p2) const {
    const double R = 6371000;

    double lat1 = p1.latitude * M_PI / 180.0;
    double lat2 = p2.latitude * M_PI / 180.0;
    double lon1 = p1.longitude * M_PI / 180.0;
    double lon2 = p2.longitude * M_PI / 180.0;

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = std::sin(dlat/2) * std::sin(dlat/2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dlon/2) * std::sin(dlon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));

    return R * c;
}

FilterResult JumpFilter::process(GpsPoint& point, const IHistory& history) {
    if (!enabled_) return FilterResult::PASS;

    if (!point.isValid) {
        return FilterResult::REJECT;
    }

    auto lastValid = history.getLastValid();
    if (!lastValid.has_value()) {
        return FilterResult::PASS;
    }

    double distance = calculateDistance(*lastValid, point);

    if (distance > maxJumpMeters_) {
        return FilterResult::REJECT;
    }

    return FilterResult::PASS;
}

void JumpFilter::setEnabled(bool enabled) { enabled_ = enabled; }
bool JumpFilter::isEnabled() const { return enabled_; }
std::string JumpFilter::getName() const { return "JumpFilter"; }
void JumpFilter::setMaxJump(double maxJump) { maxJumpMeters_ = maxJump; }
double JumpFilter::getMaxJump() const { return maxJumpMeters_; }
