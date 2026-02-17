#include "infra/file_display.h"
#include <sstream>
#include <filesystem>
#include <iostream>

FileDisplay::FileDisplay(const std::string& filename, bool rotate, size_t maxSize)
    : filename_(filename)
    , rotate_(rotate)
    , maxSize_(maxSize)
    , currentSize_(0) {

    file_.open(filename, std::ios::app);
    if (!file_.is_open()) {
        std::cerr << "Warning: Cannot open file " << filename << " for writing\n";
    }
}

FileDisplay::~FileDisplay() {
    if (file_.is_open()) {
        file_.close();
    }
}

std::string FileDisplay::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y%m%d_%H%M%S");
    ss << "_" << std::setfill('0') << std::setw(3) << now_ms.count();
    return ss.str();
}

void FileDisplay::rotateFile() {
    if (!rotate_) return;

    if (file_.is_open()) {
        file_.close();
    }

    std::string newFilename = filename_ + "." + getCurrentTimestamp();
    std::filesystem::rename(filename_, newFilename);

    file_.open(filename_, std::ios::app);
    currentSize_ = 0;
}

void FileDisplay::checkRotation() {
    if (!rotate_ || !file_.is_open()) return;

    currentSize_ = file_.tellp();
    if (currentSize_ >= maxSize_) {
        rotateFile();
    }
}

std::string FileDisplay::formatTime(unsigned long long timestampMs) const {
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

std::string FileDisplay::formatLatitude(double lat) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5) << std::fabs(lat);
    oss << (lat >= 0 ? "\xC2\xB0" "N" : "\xC2\xB0" "S");
    return oss.str();
}

std::string FileDisplay::formatLongitude(double lon) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5) << std::fabs(lon);
    oss << (lon >= 0 ? "\xC2\xB0" "E" : "\xC2\xB0" "W");
    return oss.str();
}

void FileDisplay::showPoint(const GpsPoint& point) {
    if (!file_.is_open()) return;

    checkRotation();

    file_ << "[" << formatTime(point.timestamp) << "] "
          << "Coordinates: " << formatLatitude(point.latitude) << ", " << formatLongitude(point.longitude) << "\n"
          << "               Speed: " << std::fixed << std::setprecision(1) << point.speed << " km/h";

    if (point.speed < 0.1) {
        file_ << " (stopped)";
    }

    file_ << ", Course: " << std::fixed << std::setprecision(1) << point.course << "\xC2\xB0" "\n"
          << "               Altitude: " << std::fixed << std::setprecision(0) << point.altitude << "m"
          << ", Satellites: " << point.satellites
          << ", HDOP: " << std::fixed << std::setprecision(1) << point.hdop << "\n";

    file_.flush();
}

void FileDisplay::showInvalidFix(unsigned long long timestamp) {
    if (!file_.is_open()) return;

    checkRotation();
    file_ << "[" << formatTime(timestamp) << "] No valid GPS fix\n";
    file_.flush();
}

void FileDisplay::showParseError(const std::string& error) {
    if (!file_.is_open()) return;

    checkRotation();
    file_ << "Parse error: " << error << "\n";
    file_.flush();
}

void FileDisplay::showRejected(const std::string& reason) {
    if (!file_.is_open()) return;

    checkRotation();
    file_ << "Point rejected: " << reason << "\n";
    file_.flush();
}

void FileDisplay::clear() {
    if (file_.is_open()) {
        file_.close();
        file_.open(filename_, std::ios::trunc);
        currentSize_ = 0;
    }
}
