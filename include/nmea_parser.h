#pragma once

#include "gps_point.h"
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <memory>

namespace nmea {

class Parser {
public:
    using ErrorCallback = std::function<void(const std::string&)>;
    
    Parser();
    ~Parser() = default;
    
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = default;
    Parser& operator=(Parser&&) = default;
    
    void setErrorCallback(ErrorCallback cb);
    std::optional<GpsPoint> parse(const std::string& line);
    void parseStream(std::istream& stream, 
                    std::function<void(const GpsPoint&)> pointCallback,
                    ErrorCallback errorCallback = nullptr);

private:
    ErrorCallback errorCallback_;
    std::optional<GpsPoint> lastRMC_;
    std::optional<GpsPoint> lastGGA_;
    
    bool validateChecksum(const std::string& nmea);
    double knotsToKmh(double knots) const;
    double nmeaToDegrees(const std::string& coord, char hem);
    std::chrono::system_clock::time_point parseTime(
        const std::string& timeStr, const std::string& dateStr = "");
    std::optional<GpsPoint> parseRMC(const std::vector<std::string>& fields);
    std::optional<GpsPoint> parseGGA(const std::vector<std::string>& fields);
};

} // namespace nmea