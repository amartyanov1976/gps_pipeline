#include "display.h"
#include <sstream>
#include <iomanip>

namespace gps {

ConsoleDisplay::ConsoleDisplay(std::ostream& out) : out_(out) {}

void ConsoleDisplay::show(const GpsPoint& point) {
    if (!point.isValid()) {
        error("No valid GPS fix");
        return;
    }
    
    std::stringstream ss;
    ss << "[" << point.formatTime() << "] ";
    
    if (point.hasPosition()) {
        ss << "Coordinates: " << point.formatCoordinates() << "\n";
        ss << "           Speed: " << std::fixed << std::setprecision(1)
           << point.getSpeed() << " km/h";
        
        if (point.getSpeed() < 0.1) {
            ss << " (stopped)";
        }
        
        ss << ", Course: " << std::setprecision(1) << point.getCourse() << "°\n";
        ss << "           Altitude: " << std::setprecision(0) 
           << point.getAltitude() << "m, Satellites: " << point.getSatellites()
           << ", HDOP: " << std::setprecision(1) << point.getHdop();
    } else {
        ss << "No position data";
    }
    
    out_ << ss.str() << std::endl;
}

void ConsoleDisplay::error(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::gmtime(&time);
    
    std::stringstream ss;
    ss << std::setfill('0')
       << "[" << std::setw(2) << tm->tm_hour << ":"
       << std::setw(2) << tm->tm_min << ":"
       << std::setw(2) << tm->tm_sec << "] "
       << message;
    
    out_ << ss.str() << std::endl;
}

void ConsoleDisplay::message(const std::string& message) {
    out_ << message << std::endl;
}

// MockDisplay
void MockDisplay::show(const GpsPoint& point) {
    lastPoint_ = point;
}

void MockDisplay::error(const std::string& message) {
    lastError_ = message;
}

void MockDisplay::message(const std::string& message) {
    lastMessage_ = message;
}

const GpsPoint& MockDisplay::getLastPoint() const {
    return lastPoint_;
}

const std::string& MockDisplay::getLastError() const {
    return lastError_;
}

const std::string& MockDisplay::getLastMessage() const {
    return lastMessage_;
}

void MockDisplay::clear() {
    lastPoint_ = GpsPoint();
    lastError_.clear();
    lastMessage_.clear();
}

} // namespace gps