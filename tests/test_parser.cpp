#include <gtest/gtest.h>
#include "infra/nmea_parser.h"
#include "domain/gps_point.h"

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser.reset();
    }

    NmeaParser parser;
};

TEST_F(ParserTest, ValidateChecksum_ValidChecksum_ReturnsTrue) {
    std::string line = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";
    EXPECT_TRUE(NmeaParser::validateChecksum(line));
}

TEST_F(ParserTest, ValidateChecksum_InvalidChecksum_ReturnsFalse) {
    std::string line = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*FF";
    EXPECT_FALSE(NmeaParser::validateChecksum(line));
}

TEST_F(ParserTest, ValidateChecksum_NoAsterisk_ReturnsFalse) {
    std::string line = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W";
    EXPECT_FALSE(NmeaParser::validateChecksum(line));
}

TEST_F(ParserTest, ConvertNmeaCoordinate_NorthHemisphere_ReturnsCorrectDegrees) {
    double result = NmeaParser::convertNmeaCoordinate(4807.038, 'N');
    EXPECT_NEAR(result, 48.1173, 0.0001);
}

TEST_F(ParserTest, ConvertNmeaCoordinate_SouthHemisphere_ReturnsNegativeDegrees) {
    double result = NmeaParser::convertNmeaCoordinate(4807.038, 'S');
    EXPECT_NEAR(result, -48.1173, 0.0001);
}

TEST_F(ParserTest, ConvertNmeaCoordinate_EastHemisphere_ReturnsCorrectDegrees) {
    double result = NmeaParser::convertNmeaCoordinate(1131.000, 'E');
    EXPECT_NEAR(result, 11.5167, 0.0001);
}

TEST_F(ParserTest, ConvertNmeaCoordinate_WestHemisphere_ReturnsNegativeDegrees) {
    double result = NmeaParser::convertNmeaCoordinate(1131.000, 'W');
    EXPECT_NEAR(result, -11.5167, 0.0001);
}

TEST_F(ParserTest, KnotsToKmh_ConvertsCorrectly) {
    double result = NmeaParser::knotsToKmh(22.4);
    EXPECT_NEAR(result, 41.4848, 0.001);
}

TEST_F(ParserTest, ParseTimeToMs_ValidTime_ReturnsMilliseconds) {
    auto result = NmeaParser::parseTimeToMs("123519");
    EXPECT_EQ(result, 12 * 3600 * 1000 + 35 * 60 * 1000 + 19 * 1000);
}

TEST_F(ParserTest, ParseLine_ValidRMCAndGGA_ReturnsGpsPoint) {
    parser.parseLine("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    auto point = parser.parseLine("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");

    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR(point->latitude, 48.1173, 0.0001);
    EXPECT_NEAR(point->longitude, 11.5167, 0.0001);
    EXPECT_NEAR(point->speed, 41.4848, 0.001);
    EXPECT_NEAR(point->course, 84.4, 0.1);
    EXPECT_NEAR(point->altitude, 545.4, 0.1);
    EXPECT_EQ(point->satellites, 8);
    EXPECT_NEAR(point->hdop, 0.9, 0.1);
    EXPECT_TRUE(point->isValid);
}

TEST_F(ParserTest, ParseLine_InvalidChecksum_ReturnsNullopt) {
    auto point = parser.parseLine("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF");
    EXPECT_FALSE(point.has_value());
}

TEST_F(ParserTest, ParseLine_InvalidStatus_ReturnsInvalidPoint) {
    parser.parseLine("$GPRMC,123519,V,4807.038,N,01131.000,E,022.4,084.4,230394,,,*2A");
    auto point = parser.parseLine("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");

    ASSERT_TRUE(point.has_value());
    EXPECT_FALSE(point->isValid);
}

TEST_F(ParserTest, ParseLine_DifferentTimestamps_NoPoint) {
    parser.parseLine("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    auto point = parser.parseLine("$GPGGA,123520,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4D");

    EXPECT_FALSE(point.has_value());
}

TEST_F(ParserTest, ImplementsIParserInterface) {
    IParser* iparser = &parser;
    iparser->reset();

    iparser->parseLine("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    auto point = iparser->parseLine("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");

    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(point->isValid);
}
