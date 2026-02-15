#include <gtest/gtest.h>
#include "satellite_filter.h"
#include "history.h"

class SatelliteFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<SatelliteFilter>(4); // min 4 satellites
        history = std::make_unique<GpsHistory>();
    }
    
    GpsPoint createPoint(int satellites, bool valid = true) {
        GpsPoint p;
        p.satellites = satellites;
        p.isValid = valid;
        return p;
    }
    
    std::unique_ptr<SatelliteFilter> filter;
    std::unique_ptr<GpsHistory> history;
};

TEST_F(SatelliteFilterTest, Process_Enabled_ValidPointWithEnoughSatellites_ReturnsPass) {
    auto point = createPoint(5);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SatelliteFilterTest, Process_Enabled_ValidPointWithNotEnoughSatellites_ReturnsReject) {
    auto point = createPoint(3);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SatelliteFilterTest, Process_Enabled_InvalidPoint_ReturnsReject) {
    auto point = createPoint(5, false);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SatelliteFilterTest, Process_Disabled_ReturnsPass) {
    filter->setEnabled(false);
    auto point = createPoint(1);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SatelliteFilterTest, SetMinSatellites_ChangesThreshold) {
    filter->setMinSatellites(2);
    
    auto point = createPoint(2);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    
    point.satellites = 1;
    result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(SatelliteFilterTest, GetMinSatellites_ReturnsCorrectValue) {
    EXPECT_EQ(filter->getMinSatellites(), 4);
    
    filter->setMinSatellites(8);
    EXPECT_EQ(filter->getMinSatellites(), 8);
}

TEST_F(SatelliteFilterTest, GetName_ReturnsCorrectName) {
    EXPECT_EQ(filter->getName(), "SatelliteFilter");
}