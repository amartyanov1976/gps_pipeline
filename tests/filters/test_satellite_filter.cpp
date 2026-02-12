#include <gtest/gtest.h>
#include "filters/satellite_filter.h"
#include "gps_point.h"

using namespace gps;

class SatelliteFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<SatelliteFilter>(4);
        
        validPoint = GpsPoint::zero();
        validPoint.setValid(true);
        validPoint.setSatellites(8);
        
        lowSatPoint = validPoint;
        lowSatPoint.setSatellites(2);
    }
    
    std::unique_ptr<SatelliteFilter> filter;
    GpsPoint validPoint;
    GpsPoint lowSatPoint;
    std::deque<GpsPoint> history;
};

TEST_F(SatelliteFilterTest, PassValidPoint) {
    auto result = filter->process(validPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SatelliteFilterTest, RejectLowSatellites) {
    auto result = filter->process(lowSatPoint, history);
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SatelliteFilterTest, PassWhenDisabled) {
    filter->setEnabled(false);
    auto result = filter->process(lowSatPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SatelliteFilterTest, PassInvalidPoint) {
    validPoint.setValid(false);
    auto result = filter->process(validPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SatelliteFilterTest, ConfigureMinSatellites) {
    filter->setMinSatellites(10);
    EXPECT_EQ(filter->getMinSatellites(), 10);
    
    auto result = filter->process(validPoint, history);
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SatelliteFilterTest, Name) {
    EXPECT_EQ(filter->name(), "SatelliteFilter");
}