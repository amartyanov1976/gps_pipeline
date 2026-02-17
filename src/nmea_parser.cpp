#include "infra/nmea_parser.h"
#include <cstring>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <iostream>

NmeaParser::NmeaParser() = default;
NmeaParser::~NmeaParser() = default;

void NmeaParser::reset() {
    lastRMC_.reset();
    lastGGA_.reset();
    lastGSV_.reset();
}

bool NmeaParser::validateChecksum(const std::string& line) {
    size_t asteriskPos = line.find('*');
    if (asteriskPos == std::string::npos || asteriskPos + 2 >= line.length()) {
        return false;
    }

    std::string data = line.substr(1, asteriskPos - 1);
    unsigned char calculated = 0;
    for (char c : data) {
        calculated ^= static_cast<unsigned char>(c);
    }

    std::string checksumStr = line.substr(asteriskPos + 1, 2);
    unsigned int expected;
    std::stringstream ss;
    ss << std::hex << checksumStr;
    ss >> expected;

    return calculated == static_cast<unsigned char>(expected);
}

double NmeaParser::convertNmeaCoordinate(double nmeaCoord, char hemisphere) {
    int degrees = static_cast<int>(nmeaCoord / 100);
    double minutes = nmeaCoord - (degrees * 100);
    double result = degrees + (minutes / 60.0);

    if (hemisphere == 'S' || hemisphere == 'W') {
        result = -result;
    }

    return result;
}

double NmeaParser::knotsToKmh(double knots) {
    return knots * 1.852;
}

unsigned long long NmeaParser::parseTimeToMs(const std::string& timeStr) {
    if (timeStr.length() < 6) return 0;

    int hours = std::stoi(timeStr.substr(0, 2));
    int minutes = std::stoi(timeStr.substr(2, 2));
    int seconds = std::stoi(timeStr.substr(4, 2));
    int milliseconds = 0;

    if (timeStr.length() > 7 && timeStr.find('.') != std::string::npos) {
        size_t dotPos = timeStr.find('.');
        if (dotPos + 1 < timeStr.length()) {
            std::string frac = timeStr.substr(dotPos + 1);
            milliseconds = std::stoi(frac) * 10;
        }
    }

    return static_cast<unsigned long long>(hours * 3600 + minutes * 60 + seconds) * 1000 + milliseconds;
}

std::vector<std::string> NmeaParser::splitFields(const std::string& line) const {
    std::vector<std::string> fields;
    size_t start = 0;
    size_t end = line.find(',');

    while (end != std::string::npos) {
        fields.push_back(line.substr(start, end - start));
        start = end + 1;
        end = line.find(',', start);
    }

    size_t asteriskPos = line.find('*', start);
    if (asteriskPos != std::string::npos) {
        fields.push_back(line.substr(start, asteriskPos - start));
    }

    return fields;
}

std::optional<nmea::RMCData> NmeaParser::parseRMC(const std::vector<std::string>& fields) {
    if (fields.size() < 12) return std::nullopt;

    nmea::RMCData data;
    data.timestamp = parseTimeToMs(fields[1]);
    data.valid = (fields[2] == "A");

    if (!fields[3].empty() && !fields[4].empty()) {
        data.latitude = std::stod(fields[3]);
        data.latHemisphere = fields[4][0];
    }

    if (!fields[5].empty() && !fields[6].empty()) {
        data.longitude = std::stod(fields[5]);
        data.lonHemisphere = fields[6][0];
    }

    if (!fields[7].empty()) {
        data.speedKnots = std::stod(fields[7]);
    }

    if (!fields[8].empty()) {
        data.course = std::stod(fields[8]);
    }

    if (fields.size() > 9 && !fields[9].empty()) {
        data.date = fields[9];
    }

    return data;
}

std::optional<nmea::GGAData> NmeaParser::parseGGA(const std::vector<std::string>& fields) {
    if (fields.size() < 14) return std::nullopt;

    nmea::GGAData data;
    data.timestamp = parseTimeToMs(fields[1]);

    if (!fields[2].empty() && !fields[3].empty()) {
        data.latitude = std::stod(fields[2]);
        data.latHemisphere = fields[3][0];
    }

    if (!fields[4].empty() && !fields[5].empty()) {
        data.longitude = std::stod(fields[4]);
        data.lonHemisphere = fields[5][0];
    }

    if (!fields[6].empty()) {
        data.quality = std::stoi(fields[6]);
    }

    if (!fields[7].empty()) {
        data.satellites = std::stoi(fields[7]);
    }

    if (!fields[8].empty()) {
        data.hdop = std::stof(fields[8]);
    }

    if (!fields[9].empty()) {
        data.altitude = std::stod(fields[9]);
    }

    if (fields.size() > 10) {
        data.altitudeUnit = fields[10];
    }

    return data;
}

std::optional<nmea::GSVData> NmeaParser::parseGSV(const std::vector<std::string>& fields) {
    if (fields.size() < 4) return std::nullopt;

    nmea::GSVData data;

    try {
        data.totalMessages = std::stoi(fields[1]);
        data.messageNumber = std::stoi(fields[2]);
        data.totalSatellites = std::stoi(fields[3]);

        for (size_t i = 4; i + 3 < fields.size(); i += 4) {
            if (!fields[i].empty()) {
                data.prn.push_back(std::stoi(fields[i]));
                data.elevation.push_back(std::stoi(fields[i+1]));
                data.azimuth.push_back(std::stoi(fields[i+2]));
                data.snr.push_back(std::stoi(fields[i+3]));
            }
        }
    } catch (...) {
        return std::nullopt;
    }

    return data;
}

std::optional<GpsPoint> NmeaParser::combineData(const nmea::RMCData& rmc, const nmea::GGAData& gga) {
    GpsPoint point;

    point.timestamp = rmc.timestamp;
    point.latitude = convertNmeaCoordinate(rmc.latitude, rmc.latHemisphere);
    point.longitude = convertNmeaCoordinate(rmc.longitude, rmc.lonHemisphere);
    point.speed = knotsToKmh(rmc.speedKnots);
    point.course = rmc.course;
    point.altitude = gga.altitude;
    point.satellites = gga.satellites;
    point.hdop = gga.hdop;
    point.isValid = rmc.valid && (gga.quality > 0);

    return point;
}

std::optional<GpsPoint> NmeaParser::parseLine(const std::string& line) {
    if (!validateChecksum(line)) {
        return std::nullopt;
    }

    std::vector<std::string> fields = splitFields(line);
    if (fields.empty()) return std::nullopt;

    std::string type = fields[0];
    GpsPoint point;
    bool parsed = false;

    if (type.length() >= 6) {
        std::string msgType = type.substr(3, 3);

        if (msgType == "RMC") {
            auto rmc = parseRMC(fields);
            if (rmc.has_value()) {
                lastRMC_ = rmc;
                point.timestamp = rmc->timestamp;
                point.latitude = convertNmeaCoordinate(rmc->latitude, rmc->latHemisphere);
                point.longitude = convertNmeaCoordinate(rmc->longitude, rmc->lonHemisphere);
                point.speed = knotsToKmh(rmc->speedKnots);
                point.course = rmc->course;
                point.isValid = rmc->valid;
                parsed = true;
            }
        }
        else if (msgType == "GGA") {
            auto gga = parseGGA(fields);
            if (gga.has_value()) {
                lastGGA_ = gga;
                point.timestamp = gga->timestamp;
                point.latitude = convertNmeaCoordinate(gga->latitude, gga->latHemisphere);
                point.longitude = convertNmeaCoordinate(gga->longitude, gga->lonHemisphere);
                point.altitude = gga->altitude;
                point.satellites = gga->satellites;
                point.hdop = gga->hdop;
                point.isValid = (gga->quality > 0);
                parsed = true;
            }
        }
        else if (msgType == "GSV") {
            auto gsv = parseGSV(fields);
            if (gsv.has_value()) {
                lastGSV_ = gsv;
            }
        }
    }

    if (lastRMC_.has_value() && lastGGA_.has_value() &&
        lastRMC_->timestamp == lastGGA_->timestamp) {
        auto combined = combineData(*lastRMC_, *lastGGA_);
        if (combined.has_value()) {
            point = *combined;
            parsed = true;
            lastRMC_.reset();
            lastGGA_.reset();
        }
    }

    if (parsed) {
        return point;
    }

    return std::nullopt;
}

std::optional<nmea::GSVData> NmeaParser::getLastGSV() const {
    return lastGSV_;
}
