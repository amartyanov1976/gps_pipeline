#include <gtest/gtest.h>
#include "nmea_parser.h"
#include <sstream>

using namespace nmea;

class NMEAParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<Parser>();
    }
    
    std::unique_ptr<Parser> parser;
};

TEST_F(NMEAParserTest, ValidRMC) {
    std::string nmea = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*6A\r\n";
    auto point = parser->parse(nmea);
    
    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR(point->getLatitude(), 48.1173, 0.0001);
    EXPECT_NEAR(point->getLongitude(), 11.5167, 0.0001);
    EXPECT_NEAR(point->getSpeed(), 41.4848, 0.1);
    EXPECT_NEAR(point->getCourse(), 84.4, 0.1);
    EXPECT_TRUE(point->isValid());
}

TEST_F(NMEAParserTest, ValidGGA) {
    std::string nmea = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*47\r\n";
    auto point = parser->parse(nmea);
    
    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR(point->getLatitude(), 48.1173, 0.0001);
    EXPECT_NEAR(point->getLongitude(), 11.5167, 0.0001);
    EXPECT_EQ(point->getSatellites(), 8);
    EXPECT_NEAR(point->getHdop(), 0.9, 0.1);
    EXPECT_NEAR(point->getAltitude(), 545.4, 0.1);
    EXPECT_TRUE(point->isValid());
}

TEST_F(NMEAParserTest, InvalidChecksum) {
    std::string nmea = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF\r\n";
    auto point = parser->parse(nmea);
    EXPECT_FALSE(point.has_value());
}

TEST_F(NMEAParserTest, InvalidStatus) {
    std::string nmea = "$GPRMC,123519,V,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3C\r\n";
    auto point = parser->parse(nmea);
    
    ASSERT_TRUE(point.has_value());
    EXPECT_FALSE(point->isValid());
}

TEST_F(NMEAParserTest, CombineRMCAndGGA) {
    std::string rmc = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*6A\r\n";
    std::string gga = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*47\r\n";
    
    parser->parse(rmc);
    auto point = parser->parse(gga);
    
    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR(point->getSpeed(), 41.4848, 0.1);
    EXPECT_EQ(point->getSatellites(), 8);
    EXPECT_NEAR(point->getAltitude(), 545.4, 0.1);
}

TEST_F(NMEAParserTest, CoordinateConversion) {
    std::string nmea = "$GPRMC,123519,A,4807.038,S,01131.000,W,022.4,084.4,230394,,,*1E\r\n";
    auto point = parser->parse(nmea);
    
    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR(point->getLatitude(), -48.1173, 0.0001);
    EXPECT_NEAR(point->getLongitude(), -11.5167, 0.0001);
}

TEST_F(NMEAParserTest, ParseStream) {
    std::stringstream ss;
    ss << "$GPRMC,120000,A,5545.1234,N,03739.5678,E,025.0,045.0,270124,,,A*5E\r\n"
       << "$GPGGA,120000,5545.1234,N,03739.5678,E,1,10,0.8,150.0,M,14.0,M,,*5E\r\n"
       << "# Comment line\n"
       << "$GPRMC,120001,V,,,,,,,270124,,,N*53\r\n";
    
    int count = 0;
    parser->parseStream(ss,
        [&count](const GpsPoint&) { count++; },
        [](const std::string&) {}
    );
    
    EXPECT_EQ(count, 1); // Only valid combined point
}

TEST_F(NMEAParserTest, ErrorCallback) {
    bool errorCalled = false;
    parser->setErrorCallback([&errorCalled](const std::string&) {
        errorCalled = true;
    });
    
    parser->parse("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF\r\n");
    EXPECT_TRUE(errorCalled);
}