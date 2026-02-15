#include "mock_display.h"

void MockDisplay::showPoint(const GpsPoint& point) {
    DisplayCall call;
    call.type = DisplayCall::Type::POINT;
    call.point = point;
    call.timestamp = point.timestamp;
    calls_.push_back(call);
}

void MockDisplay::showInvalidFix(unsigned long long timestamp) {
    DisplayCall call;
    call.type = DisplayCall::Type::INVALID_FIX;
    call.timestamp = timestamp;
    calls_.push_back(call);
}

void MockDisplay::showParseError(const std::string& error) {
    DisplayCall call;
    call.type = DisplayCall::Type::PARSE_ERROR;
    call.message = error;
    calls_.push_back(call);
}

void MockDisplay::showRejected(const std::string& reason) {
    DisplayCall call;
    call.type = DisplayCall::Type::REJECTED;
    call.message = reason;
    calls_.push_back(call);
}

void MockDisplay::clear() {
    calls_.clear();
}

const std::vector<DisplayCall>& MockDisplay::getCalls() const {
    return calls_;
}

void MockDisplay::reset() {
    calls_.clear();
}

bool MockDisplay::hasPointWithTime(unsigned long long timestamp) const {
    for (const auto& call : calls_) {
        if (call.type == DisplayCall::Type::POINT && call.timestamp == timestamp) {
            return true;
        }
    }
    return false;
}

int MockDisplay::getPointCount() const {
    int count = 0;
    for (const auto& call : calls_) {
        if (call.type == DisplayCall::Type::POINT) {
            count++;
        }
    }
    return count;
}

int MockDisplay::getInvalidFixCount() const {
    int count = 0;
    for (const auto& call : calls_) {
        if (call.type == DisplayCall::Type::INVALID_FIX) {
            count++;
        }
    }
    return count;
}

int MockDisplay::getErrorCount() const {
    int count = 0;
    for (const auto& call : calls_) {
        if (call.type == DisplayCall::Type::PARSE_ERROR || 
            call.type == DisplayCall::Type::REJECTED) {
            count++;
        }
    }
    return count;
}