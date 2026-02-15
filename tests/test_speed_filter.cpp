#include <gtest/gtest.h>
#include "speed_filter.h"
#include "history.h"

class SpeedFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<SpeedFilter>(300.0); // max 300 km/h
        history = std::make_unique<GpsHistory>();
    }
    
    GpsPoint createPoint(double speed, bool valid = true) {
        GpsPoint p;
        p.speed = speed;
        p.isValid = valid;
        return p;
    }
    
    std::unique_ptr<SpeedFilter> filter;
    std::unique_ptr<GpsHistory> history;
};

TEST_F(SpeedFilterTest, Process_Enabled_ValidSpeed_ReturnsPass) {
    auto point = createPoint(120.5);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SpeedFilterTest, Process_Enabled_SpeedTooHigh_ReturnsReject) {
    auto point = createPoint(350.0);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SpeedFilterTest, Process_Enabled_NegativeSpeed_ReturnsReject) {
    auto point = createPoint(-10.0);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SpeedFilterTest, Process_Enabled_InvalidPoint_ReturnsReject) {
    auto point = createPoint(120.5, false);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SpeedFilterTest, Process_Disabled_ReturnsPass) {
    filter->setEnabled(false);
    auto point = createPoint(500.0);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SpeedFilterTest, SetMaxSpeed_ChangesThreshold) {
    filter->setMaxSpeed(50.0);
    
    auto point = createPoint(49.9);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    
    point.speed = 50.1;
    result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SpeedFilterTest, GetMaxSpeed_ReturnsCorrectValue) {
    EXPECT_EQ(filter->getMaxSpeed(), 300.0);
    
    filter->setMaxSpeed(150.0);
    EXPECT_EQ(filter->getMaxSpeed(), 150.0);
}

TEST_F(SpeedFilterTest, GetName_ReturnsCorrectName) {
    EXPECT_EQ(filter->getName(), "SpeedFilter");
}