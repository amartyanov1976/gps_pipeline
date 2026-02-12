#include <gtest/gtest.h>
#include "filters/speed_filter.h"
#include "gps_point.h"

using namespace gps;

class SpeedFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<SpeedFilter>(200.0);
        
        validPoint = GpsPoint::zero();
        validPoint.setValid(true);
        validPoint.setSpeed(150.0);
        
        highSpeedPoint = validPoint;
        highSpeedPoint.setSpeed(250.0);
        
        boundaryPoint = validPoint;
        boundaryPoint.setSpeed(200.0);
    }
    
    std::unique_ptr<SpeedFilter> filter;
    GpsPoint validPoint;
    GpsPoint highSpeedPoint;
    GpsPoint boundaryPoint;
    std::deque<GpsPoint> history;
};

TEST_F(SpeedFilterTest, PassValidSpeed) {
    auto result = filter->process(validPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SpeedFilterTest, RejectHighSpeed) {
    auto result = filter->process(highSpeedPoint, history);
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SpeedFilterTest, PassBoundarySpeed) {
    auto result = filter->process(boundaryPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SpeedFilterTest, PassWhenDisabled) {
    filter->setEnabled(false);
    auto result = filter->process(highSpeedPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SpeedFilterTest, ConfigureMaxSpeed) {
    filter->setMaxSpeed(100.0);
    EXPECT_EQ(filter->getMaxSpeed(), 100.0);
    
    auto result = filter->process(validPoint, history);
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SpeedFilterTest, Name) {
    EXPECT_EQ(filter->name(), "SpeedFilter");
}