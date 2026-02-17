#include "infra/console_display.h"
#include <iomanip>

ConsoleDisplay::ConsoleDisplay(std::ostream& output) : out_(output) {}

std::string ConsoleDisplay::formatTime(unsigned long long timestampMs) const {
    unsigned long long seconds = timestampMs / 1000;
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << secs;
    return oss.str();
}

std::string ConsoleDisplay::formatLatitude(double lat) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5) << std::fabs(lat);
    oss << (lat >= 0 ? "\xC2\xB0" "N" : "\xC2\xB0" "S");
    return oss.str();
}

std::string ConsoleDisplay::formatLongitude(double lon) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5) << std::fabs(lon);
    oss << (lon >= 0 ? "\xC2\xB0" "E" : "\xC2\xB0" "W");
    return oss.str();
}

void ConsoleDisplay::showPoint(const GpsPoint& point) {
    out_ << "[" << formatTime(point.timestamp) << "] "
         << "Coordinates: " << formatLatitude(point.latitude) << ", " << formatLongitude(point.longitude) << "\n"
         << "               Speed: " << std::fixed << std::setprecision(1) << point.speed << " km/h";

    if (point.speed < 0.1) {
        out_ << " (stopped)";
    }

    out_ << ", Course: " << std::fixed << std::setprecision(1) << point.course << "\xC2\xB0" "\n"
         << "               Altitude: " << std::fixed << std::setprecision(0) << point.altitude << "m"
         << ", Satellites: " << point.satellites
         << ", HDOP: " << std::fixed << std::setprecision(1) << point.hdop << "\n";
}

void ConsoleDisplay::showInvalidFix(unsigned long long timestamp) {
    out_ << "[" << formatTime(timestamp) << "] No valid GPS fix\n";
}

void ConsoleDisplay::showParseError(const std::string& error) {
    out_ << "Parse error: " << error << "\n";
}

void ConsoleDisplay::showRejected(const std::string& reason) {
    out_ << "Point rejected: " << reason << "\n";
}

void ConsoleDisplay::clear() {}
