#include <gtest/gtest.h>
#include "console_display.h"
#include <sstream>

class ConsoleDisplayTest : public ::testing::Test {
protected:
    void SetUp() override {
        output.str("");
        display = std::make_unique<ConsoleDisplay>(output);
    }
    
    GpsPoint createValidPoint() {
        GpsPoint p;
        p.latitude = 48.1173;
        p.longitude = 11.5167;
        p.speed = 41.5;
        p.course = 84.4;
        p.altitude = 545.4;
        p.satellites = 8;
        p.hdop = 0.9f;
        p.timestamp = 12 * 3600 * 1000 + 35 * 60 * 1000 + 19 * 1000; // 12:35:19
        p.isValid = true;
        return p;
    }
    
    std::stringstream output;
    std::unique_ptr<ConsoleDisplay> display;
};

TEST_F(ConsoleDisplayTest, ShowPoint_ValidPoint_FormatsCorrectly) {
    auto point = createValidPoint();
    
    display->showPoint(point);
    
    std::string result = output.str();
    EXPECT_NE(result.find("[12:35:19]"), std::string::npos);
    EXPECT_NE(result.find("48.11730°N"), std::string::npos);
    EXPECT_NE(result.find("11.51670°E"), std::string::npos);
    EXPECT_NE(result.find("41.5 km/h"), std::string::npos);
    EXPECT_NE(result.find("84.4°"), std::string::npos);
    EXPECT_NE(result.find("545m"), std::string::npos);
    EXPECT_NE(result.find("Satellites: 8"), std::string::npos);
    EXPECT_NE(result.find("HDOP: 0.9"), std::string::npos);
}

TEST_F(ConsoleDisplayTest, ShowPoint_ZeroSpeed_ShowsStopped) {
    auto point = createValidPoint();
    point.speed = 0.0;
    
    display->showPoint(point);
    
    std::string result = output.str();
    EXPECT_NE(result.find("(stopped)"), std::string::npos);
}

TEST_F(ConsoleDisplayTest, ShowPoint_NegativeLatitude_ShowsSouth) {
    auto point = createValidPoint();
    point.latitude = -48.1173;
    
    display->showPoint(point);
    
    std::string result = output.str();
    EXPECT_NE(result.find("48.11730°S"), std::string::npos);
}

TEST_F(ConsoleDisplayTest, ShowPoint_NegativeLongitude_ShowsWest) {
    auto point = createValidPoint();
    point.longitude = -11.5167;
    
    display->showPoint(point);
    
    std::string result = output.str();
    EXPECT_NE(result.find("11.51670°W"), std::string::npos);
}

TEST_F(ConsoleDisplayTest, ShowInvalidFix_FormatsCorrectly) {
    display->showInvalidFix(12 * 3600 * 1000 + 35 * 60 * 1000 + 20 * 1000);
    
    std::string result = output.str();
    EXPECT_EQ(result, "[12:35:20] No valid GPS fix\n");
}

TEST_F(ConsoleDisplayTest, ShowParseError_FormatsCorrectly) {
    display->showParseError("invalid checksum");
    
    std::string result = output.str();
    EXPECT_EQ(result, "Parse error: invalid checksum\n");
}

TEST_F(ConsoleDisplayTest, ShowRejected_FormatsCorrectly) {
    display->showRejected("insufficient satellites");
    
    std::string result = output.str();
    EXPECT_EQ(result, "Point rejected: insufficient satellites\n");
}

TEST_F(ConsoleDisplayTest, Clear_DoesNothing) {
    // Просто проверяем, что не падает
    display->clear();
    EXPECT_TRUE(output.str().empty());
}