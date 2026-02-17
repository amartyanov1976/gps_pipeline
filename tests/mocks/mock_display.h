#pragma once

#include "domain/i_display.h"
#include <vector>
#include <string>

struct DisplayCall {
    enum class Type {
        POINT,
        INVALID_FIX,
        PARSE_ERROR,
        REJECTED
    };

    Type type;
    GpsPoint point;
    unsigned long long timestamp = 0;
    std::string message;
};

class MockDisplay : public IDisplay {
public:
    void showPoint(const GpsPoint& point) override {
        DisplayCall call;
        call.type = DisplayCall::Type::POINT;
        call.point = point;
        call.timestamp = point.timestamp;
        calls_.push_back(call);
    }

    void showInvalidFix(unsigned long long timestamp) override {
        DisplayCall call;
        call.type = DisplayCall::Type::INVALID_FIX;
        call.timestamp = timestamp;
        calls_.push_back(call);
    }

    void showParseError(const std::string& error) override {
        DisplayCall call;
        call.type = DisplayCall::Type::PARSE_ERROR;
        call.message = error;
        calls_.push_back(call);
    }

    void showRejected(const std::string& reason) override {
        DisplayCall call;
        call.type = DisplayCall::Type::REJECTED;
        call.message = reason;
        calls_.push_back(call);
    }

    void clear() override {
        calls_.clear();
    }

    const std::vector<DisplayCall>& getCalls() const { return calls_; }

    void reset() { calls_.clear(); }

    bool hasPointWithTime(unsigned long long timestamp) const {
        for (const auto& call : calls_) {
            if (call.type == DisplayCall::Type::POINT && call.timestamp == timestamp) {
                return true;
            }
        }
        return false;
    }

    int getPointCount() const {
        int count = 0;
        for (const auto& call : calls_) {
            if (call.type == DisplayCall::Type::POINT) count++;
        }
        return count;
    }

    int getInvalidFixCount() const {
        int count = 0;
        for (const auto& call : calls_) {
            if (call.type == DisplayCall::Type::INVALID_FIX) count++;
        }
        return count;
    }

    int getErrorCount() const {
        int count = 0;
        for (const auto& call : calls_) {
            if (call.type == DisplayCall::Type::PARSE_ERROR ||
                call.type == DisplayCall::Type::REJECTED) count++;
        }
        return count;
    }

private:
    std::vector<DisplayCall> calls_;
};
