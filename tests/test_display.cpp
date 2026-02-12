#include <gtest/gtest.h>
#include "display.h"
#include <sstream>

using namespace gps;

TEST(ConsoleDisplayTest, ShowValidPoint) {
    std::stringstream output;
    ConsoleDisplay display(output);
    
    auto now = std::chrono::system_clock::now();
    GpsPoint point(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8, now, true);
    
    display.show(point);
    std::string result = output.str();
    
    EXPECT_TRUE(result.find("Coordinates") != std::string::npos);
    EXPECT_TRUE(result.find("55.75206") != std::string::npos);
    EXPECT_TRUE(result.find("46.3") != std::string::npos);
}

TEST(ConsoleDisplayTest, ShowInvalidPoint) {
    std::stringstream output;
    ConsoleDisplay display(output);
    
    GpsPoint point = GpsPoint::invalid();
    display.show(point);
    
    std::string result = output.str();
    EXPECT_TRUE(result.find("No valid GPS fix") != std::string::npos);
}

TEST(ConsoleDisplayTest, ShowStoppedPoint) {
    std::stringstream output;
    ConsoleDisplay display(output);
    
    auto now = std::chrono::system_clock::now();
    GpsPoint point(55.752056, 37.659463, 0.0, 45.0, 150.0, 10, 0.8, now, true);
    
    display.show(point);
    std::string result = output.str();
    
    EXPECT_TRUE(result.find("stopped") != std::string::npos);
}

TEST(ConsoleDisplayTest, ErrorMessage) {
    std::stringstream output;
    ConsoleDisplay display(output);
    
    display.error("Test error message");
    std::string result = output.str();
    
    EXPECT_TRUE(result.find("Test error message") != std::string::npos);
}

TEST(MockDisplayTest, RecordPoint) {
    MockDisplay display;
    
    auto now = std::chrono::system_clock::now();
    GpsPoint point(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8, now, true);
    
    display.show(point);
    EXPECT_EQ(display.getLastPoint(), point);
}

TEST(MockDisplayTest, RecordError) {
    MockDisplay display;
    
    display.error("Test error");
    EXPECT_EQ(display.getLastError(), "Test error");
}

TEST(MockDisplayTest, Clear) {
    MockDisplay display;
    
    display.error("Test error");
    display.clear();
    
    EXPECT_TRUE(display.getLastError().empty());
}