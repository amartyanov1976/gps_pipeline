#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "domain/i_parser.h"
#include "domain/gps_point.h"

namespace nmea {
    struct RMCData {
        unsigned long long timestamp = 0;
        bool valid = false;
        double latitude = 0.0;
        char latHemisphere = 'N';
        double longitude = 0.0;
        char lonHemisphere = 'E';
        double speedKnots = 0.0;
        double course = 0.0;
        std::string date;
        std::string magneticVariation;
        char magVariationDir = 'E';
    };

    struct GGAData {
        unsigned long long timestamp = 0;
        double latitude = 0.0;
        char latHemisphere = 'N';
        double longitude = 0.0;
        char lonHemisphere = 'E';
        int quality = 0;
        int satellites = 0;
        float hdop = 0.0f;
        double altitude = 0.0;
        std::string altitudeUnit = "M";
        double geoidalSeparation = 0.0;
        std::string geoidalUnit = "M";
    };

    struct GSVData {
        int totalMessages = 0;
        int messageNumber = 0;
        int totalSatellites = 0;
        std::vector<int> prn;
        std::vector<int> elevation;
        std::vector<int> azimuth;
        std::vector<int> snr;
    };
}

class NmeaParser : public IParser {
public:
    NmeaParser();
    ~NmeaParser() override;

    std::optional<GpsPoint> parseLine(const std::string& line) override;
    void reset() override;

    std::optional<nmea::GSVData> getLastGSV() const;

    static bool validateChecksum(const std::string& line);
    static double convertNmeaCoordinate(double nmeaCoord, char hemisphere);
    static double knotsToKmh(double knots);
    static unsigned long long parseTimeToMs(const std::string& timeStr);

private:
    std::string extractChecksumPart(const std::string& line) const;
    unsigned char calculateChecksum(const std::string& data) const;
    std::vector<std::string> splitFields(const std::string& line) const;

    std::optional<nmea::RMCData> parseRMC(const std::vector<std::string>& fields);
    std::optional<nmea::GGAData> parseGGA(const std::vector<std::string>& fields);
    std::optional<nmea::GSVData> parseGSV(const std::vector<std::string>& fields);
    std::optional<GpsPoint> combineData(const nmea::RMCData& rmc, const nmea::GGAData& gga);

    std::optional<nmea::RMCData> lastRMC_;
    std::optional<nmea::GGAData> lastGGA_;
    std::optional<nmea::GSVData> lastGSV_;
};
