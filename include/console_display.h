#pragma once

#include "display_interface.h"
#include <iostream>
#include <iomanip>
#include <sstream>

class ConsoleDisplay : public IDisplay {
public:
    ConsoleDisplay(std::ostream& output = std::cout);
    
    void showPoint(const GpsPoint& point) override;
    void showInvalidFix(unsigned long long timestamp) override;
    void showParseError(const std::string& error) override;
    void showRejected(const std::string& reason) override;
    void clear() override;
    
private:
    std::string formatTime(unsigned long long timestampMs) const;
    std::string formatLatitude(double lat) const;
    std::string formatLongitude(double lon) const;
    
    std::ostream& out_;
};