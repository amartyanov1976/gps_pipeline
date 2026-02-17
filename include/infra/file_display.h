#pragma once

#include "domain/i_display.h"
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>

class FileDisplay : public IDisplay {
public:
    FileDisplay(const std::string& filename, bool rotate = false, size_t maxSize = 1024 * 1024);
    ~FileDisplay() override;

    void showPoint(const GpsPoint& point) override;
    void showInvalidFix(unsigned long long timestamp) override;
    void showParseError(const std::string& error) override;
    void showRejected(const std::string& reason) override;
    void clear() override;

private:
    void checkRotation();
    std::string formatTime(unsigned long long timestampMs) const;
    std::string formatLatitude(double lat) const;
    std::string formatLongitude(double lon) const;
    std::string getCurrentTimestamp();
    void rotateFile();

    std::string filename_;
    std::ofstream file_;
    bool rotate_;
    size_t maxSize_;
    size_t currentSize_;
};
