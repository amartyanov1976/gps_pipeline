#include <gtest/gtest.h>
#include "filters/jump_filter.h"
#include "gps_point.h"
#include <chrono>

using namespace gps;

class JumpFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<JumpFilter>(100.0);
        
        basePoint = GpsPoint::zero();
        basePoint.setValid(true);
        basePoint.setLatitude(55.752056);
        basePoint.setLongitude(37.659463);
        basePoint.setTimestamp(std::chrono::system_clock::now());
        
        smallJump = basePoint;
        smallJump.setLatitude(55.752156); // ~11 метров
        smallJump.setTimestamp(basePoint.getTimestamp() + std::chrono::seconds(1));
        
        largeJump = basePoint;
        largeJump.setLatitude(55.762056); // ~1.1 км
        largeJump.setTimestamp(basePoint.getTimestamp() + std::chrono::seconds(1));
        
        history.push_back(basePoint);
    }
    
    std::unique_ptr<JumpFilter> filter;
    GpsPoint basePoint;
    GpsPoint smallJump;
    GpsPoint largeJump;
    std::deque<GpsPoint> history;
};

TEST_F(JumpFilterTest, PassSmallJump) {
    auto result = filter->process(smallJump, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(JumpFilterTest, RejectLargeJump) {
    auto result = filter->process(largeJump, history);
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(JumpFilterTest, PassWhenNoHistory) {
    history.clear();
    auto result = filter->process(largeJump, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(JumpFilterTest, PassInvalidPoint) {
    largeJump.setValid(false);
    auto result = filter->process(largeJump, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(JumpFilterTest, ConfigureMaxJump) {
    filter->setMaxJump(50.0);
    EXPECT_EQ(filter->getMaxJump(), 50.0);
    
    auto result = filter->process(smallJump, history);
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(JumpFilterTest, Name) {
    EXPECT_EQ(filter->name(), "JumpFilter");
}