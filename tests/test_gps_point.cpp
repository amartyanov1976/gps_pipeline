#include <gtest/gtest.h>
#include "gps_point.h"
#include <cmath>

TEST(GpsPointTest, DefaultConstructor) {
    GpsPoint p;
    EXPECT_NEAR(p.getLatitude(), 0.0, 0.0001);
    EXPECT_NEAR(p.getLongitude(), 0.0, 0.0001);
    EXPECT_NEAR(p.getSpeed(), 0.0, 0.0001);
    EXPECT_NEAR(p.getCourse(), 0.0, 0.0001);
    EXPECT_NEAR(p.getAltitude(), 0.0, 0.0001);
    EXPECT_EQ(p.getSatellites(), 0);
    EXPECT_NEAR(p.getHdop(), 0.0, 0.0001);
    EXPECT_FALSE(p.isValid());
    EXPECT_FALSE(p.hasPosition());
}

TEST(GpsPointTest, ParameterizedConstructor) {
    auto now = std::chrono::system_clock::now();
    GpsPoint p(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8, now, true);
    
    EXPECT_NEAR(p.getLatitude(), 55.752056, 0.0001);
    EXPECT_NEAR(p.getLongitude(), 37.659463, 0.0001);
    EXPECT_NEAR(p.getSpeed(), 46.3, 0.1);
    EXPECT_NEAR(p.getCourse(), 45.0, 0.1);
    EXPECT_NEAR(p.getAltitude(), 150.0, 0.1);
    EXPECT_EQ(p.getSatellites(), 10);
    EXPECT_NEAR(p.getHdop(), 0.8, 0.1);
    EXPECT_EQ(p.getTimestamp(), now);
    EXPECT_TRUE(p.isValid());
    EXPECT_TRUE(p.hasPosition());
}

TEST(GpsPointTest, CopyConstructor) {
    auto now = std::chrono::system_clock::now();
    GpsPoint p1(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8, now, true);
    GpsPoint p2(p1);
    
    EXPECT_EQ(p1, p2);
}

TEST(GpsPointTest, MoveConstructor) {
    auto now = std::chrono::system_clock::now();
    GpsPoint p1(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8, now, true);
    GpsPoint p2(std::move(p1));
    
    EXPECT_NEAR(p2.getLatitude(), 55.752056, 0.0001);
    EXPECT_NEAR(p1.getLatitude(), 0.0, 0.0001); // Moved-from state
}

TEST(GpsPointTest, AssignmentOperator) {
    auto now = std::chrono::system_clock::now();
    GpsPoint p1(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8, now, true);
    GpsPoint p2;
    p2 = p1;
    
    EXPECT_EQ(p1, p2);
}

TEST(GpsPointTest, MoveAssignment) {
    auto now = std::chrono::system_clock::now();
    GpsPoint p1(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8, now, true);
    GpsPoint p2;
    p2 = std::move(p1);
    
    EXPECT_NEAR(p2.getLatitude(), 55.752056, 0.0001);
}

TEST(GpsPointTest, DistanceCalculation) {
    GpsPoint p1(55.752056, 37.659463, 0, 0, 0, 0, 0, 
                std::chrono::system_clock::now(), true);
    GpsPoint p2(55.752156, 37.659463, 0, 0, 0, 0, 0,
                std::chrono::system_clock::now(), true);
    
    double distance = p1.distanceTo(p2);
    EXPECT_NEAR(distance, 11.1, 1.0); // ~11 метров
    
    p2.setLatitude(55.762056);
    distance = p1.distanceTo(p2);
    EXPECT_NEAR(distance, 1112.0, 10.0); // ~1.1 км
}

TEST(GpsPointTest, FormatTime) {
    std::chrono::system_clock::time_point time;
    GpsPoint p(0, 0, 0, 0, 0, 0, 0, time, true);
    
    std::string formatted = p.formatTime();
    EXPECT_FALSE(formatted.empty());
}

TEST(GpsPointTest, FormatCoordinates) {
    GpsPoint p(55.752056, 37.659463, 0, 0, 0, 0, 0,
               std::chrono::system_clock::now(), true);
    
    std::string formatted = p.formatCoordinates();
    EXPECT_TRUE(formatted.find("N") != std::string::npos);
    EXPECT_TRUE(formatted.find("E") != std::string::npos);
}

TEST(GpsPointTest, ToString) {
    GpsPoint p(55.752056, 37.659463, 46.3, 45.0, 150.0, 10, 0.8,
               std::chrono::system_clock::now(), true);
    
    std::string str = p.toString();
    EXPECT_TRUE(str.find("GpsPoint") != std::string::npos);
    EXPECT_TRUE(str.find("55.75206") != std::string::npos);
    
    p.setValid(false);
    str = p.toString();
    EXPECT_TRUE(str.find("INVALID") != std::string::npos);
}

TEST(GpsPointTest, StaticFactories) {
    auto invalid = GpsPoint::invalid();
    EXPECT_FALSE(invalid.isValid());
    
    auto zero = GpsPoint::zero();
    EXPECT_TRUE(zero.isValid());
    EXPECT_NEAR(zero.getLatitude(), 0.0, 0.0001);
}

TEST(GpsPointTest, Validation) {
    GpsPoint p;
    
    EXPECT_THROW(p.setLatitude(100.0), std::out_of_range);
    EXPECT_THROW(p.setLatitude(-100.0), std::out_of_range);
    EXPECT_THROW(p.setLongitude(200.0), std::out_of_range);
    EXPECT_THROW(p.setLongitude(-200.0), std::out_of_range);
    
    p.setCourse(370.0);
    EXPECT_NEAR(p.getCourse(), 10.0, 0.1);
    
    p.setCourse(-10.0);
    EXPECT_NEAR(p.getCourse(), 350.0, 0.1);
}