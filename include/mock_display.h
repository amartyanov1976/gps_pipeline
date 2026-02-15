#pragma once

#include "display_interface.h"
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
    void showPoint(const GpsPoint& point) override;
    void showInvalidFix(unsigned long long timestamp) override;
    void showParseError(const std::string& error) override;
    void showRejected(const std::string& reason) override;
    void clear() override;
    
    // Методы для тестирования
    const std::vector<DisplayCall>& getCalls() const;
    void reset();
    bool hasPointWithTime(unsigned long long timestamp) const;
    int getPointCount() const;
    int getInvalidFixCount() const;
    int getErrorCount() const;
    
private:
    std::vector<DisplayCall> calls_;
};