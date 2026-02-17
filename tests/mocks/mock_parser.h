#pragma once

#include "domain/i_parser.h"
#include <vector>
#include <optional>

class MockParser : public IParser {
public:
    std::optional<GpsPoint> parseLine(const std::string& line) override {
        calls_.push_back(line);
        if (results_.empty()) return std::nullopt;
        auto result = results_.front();
        results_.erase(results_.begin());
        return result;
    }

    void reset() override {
        resetCalled_ = true;
    }

    void enqueueResult(std::optional<GpsPoint> result) {
        results_.push_back(result);
    }

    const std::vector<std::string>& getCalls() const { return calls_; }
    bool wasResetCalled() const { return resetCalled_; }

private:
    std::vector<std::optional<GpsPoint>> results_;
    std::vector<std::string> calls_;
    bool resetCalled_ = false;
};
