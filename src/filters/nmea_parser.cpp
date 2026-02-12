#include "nmea_parser.h"
#include <sstream>
#include <iomanip>
#include <iostream>

namespace nmea {

Parser::Parser() = default;

void Parser::setErrorCallback(ErrorCallback cb) {
    errorCallback_ = cb;
}

bool Parser::validateChecksum(const std::string& nmea) {
    size_t asteriskPos = nmea.find('*');
    if (asteriskPos == std::string::npos || asteriskPos + 2 >= nmea.length()) {
        return false;
    }
    
    std::string checksumStr = nmea.substr(asteriskPos + 1, 2);
    uint8_t expected;
    try {
        expected = static_cast<uint8_t>(std::stoi(checksumStr, nullptr, 16));
    } catch (...) {
        return false;
    }
    
    uint8_t calculated = 0;
    for (size_t i = 1; i < asteriskPos; ++i) {
        calculated ^= static_cast<uint8_t>(nmea[i]);
    }
    
    return calculated == expected;
}

double Parser::knotsToKmh(double knots) const {
    return knots * 1.852;
}

double Parser::nmeaToDegrees(const std::string& coord, char hem) {
    if (coord.empty()) return 0.0;
    
    try {
        double value = std::stod(coord);
        int degrees = static_cast<int>(value / 100);
        double minutes = value - degrees * 100;
        double result = degrees + minutes / 60.0;
        
        if (hem == 'S' || hem == 'W') {
            result = -result;
        }
        
        return result;
    } catch (...) {
        return 0.0;
    }
}

std::chrono::system_clock::time_point Parser::parseTime(
    const std::string& timeStr, const std::string& dateStr) {
    if (timeStr.empty()) {
        return std::chrono::system_clock::now();
    }
    
    try {
        int hours = std::stoi(timeStr.substr(0, 2));
        int minutes = std::stoi(timeStr.substr(2, 2));
        int seconds = std::stoi(timeStr.substr(4, 2));
        
        auto now = std::chrono::system_clock::now();
        auto nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm* tm = std::gmtime(&nowTime);
        
        tm->tm_hour = hours;
        tm->tm_min = minutes;
        tm->tm_sec = seconds;
        
        if (!dateStr.empty() && dateStr.length() == 6) {
            int day = std::stoi(dateStr.substr(0, 2));
            int month = std::stoi(dateStr.substr(2, 2));
            int year = std::stoi(dateStr.substr(4, 2)) + 2000;
            
            tm->tm_mday = day;
            tm->tm_mon = month - 1;
            tm->tm_year = year - 1900;
        }
        
        return std::chrono::system_clock::from_time_t(std::mktime(tm));
    } catch (...) {
        return std::chrono::system_clock::now();
    }
}

std::optional<GpsPoint> Parser::parseRMC(const std::vector<std::string>& fields) {
    if (fields.size() < 12) return std::nullopt;
    
    GpsPoint point;
    point.setTimestamp(parseTime(fields[1], fields[9]));
    point.setValid(fields[2] == "A");
    
    if (point.isValid()) {
        if (fields[3].size() && fields[4].size()) {
            point.setLatitude(nmeaToDegrees(fields[3], fields[4][0]));
        }
        if (fields[5].size() && fields[6].size()) {
            point.setLongitude(nmeaToDegrees(fields[5], fields[6][0]));
        }
        if (fields[7].size()) {
            point.setSpeed(knotsToKmh(std::stod(fields[7])));
        }
        if (fields[8].size()) {
            point.setCourse(std::stod(fields[8]));
        }
    }
    
    lastRMC_ = point;
    
    if (lastGGA_ && lastGGA_->getTimestamp() == point.getTimestamp()) {
        GpsPoint combined = point;
        combined.setAltitude(lastGGA_->getAltitude());
        combined.setSatellites(lastGGA_->getSatellites());
        combined.setHdop(lastGGA_->getHdop());
        lastRMC_.reset();
        lastGGA_.reset();
        return combined;
    }
    
    return std::nullopt;
}

std::optional<GpsPoint> Parser::parseGGA(const std::vector<std::string>& fields) {
    if (fields.size() < 15) return std::nullopt;
    
    GpsPoint point;
    point.setTimestamp(parseTime(fields[1]));
    
    std::string fixQuality = fields[6];
    point.setValid(fixQuality == "1" || fixQuality == "2");
    
    if (fields[2].size() && fields[3].size()) {
        point.setLatitude(nmeaToDegrees(fields[2], fields[3][0]));
    }
    if (fields[4].size() && fields[5].size()) {
        point.setLongitude(nmeaToDegrees(fields[4], fields[5][0]));
    }
    if (fields[7].size()) {
        point.setSatellites(std::stoi(fields[7]));
    }
    if (fields[8].size()) {
        point.setHdop(std::stod(fields[8]));
    }
    if (fields[9].size()) {
        point.setAltitude(std::stod(fields[9]));
    }
    
    lastGGA_ = point;
    
    if (lastRMC_ && lastRMC_->getTimestamp() == point.getTimestamp()) {
        GpsPoint combined = *lastRMC_;
        combined.setAltitude(point.getAltitude());
        combined.setSatellites(point.getSatellites());
        combined.setHdop(point.getHdop());
        lastRMC_.reset();
        lastGGA_.reset();
        return combined;
    }
    
    return std::nullopt;
}

std::optional<GpsPoint> Parser::parse(const std::string& line) {
    if (line.empty() || line[0] != '$') {
        return std::nullopt;
    }
    
    std::string cleanLine = line;
    if (!cleanLine.empty() && cleanLine.back() == '\n') cleanLine.pop_back();
    if (!cleanLine.empty() && cleanLine.back() == '\r') cleanLine.pop_back();
    
    if (!validateChecksum(cleanLine)) {
        if (errorCallback_) errorCallback_("Invalid checksum: " + line);
        return std::nullopt;
    }
    
    size_t asteriskPos = cleanLine.find('*');
    if (asteriskPos == std::string::npos) return std::nullopt;
    
    std::string data = cleanLine.substr(1, asteriskPos - 1);
    std::vector<std::string> fields;
    std::stringstream ss(data);
    std::string field;
    
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    
    if (fields.empty()) return std::nullopt;
    
    std::string type = fields[0].substr(2);
    
    if (type == "RMC") {
        return parseRMC(fields);
    } else if (type == "GGA") {
        return parseGGA(fields);
    }
    
    return std::nullopt;
}

void Parser::parseStream(std::istream& stream,
                        std::function<void(const GpsPoint&)> pointCallback,
                        ErrorCallback errorCallback) {
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        auto point = parse(line);
        if (point) {
            pointCallback(*point);
        } else if (errorCallback) {
            errorCallback("Failed to parse: " + line);
        }
    }
}

} // namespace nmea