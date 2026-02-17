#include <gtest/gtest.h>
#include "infra/stop_filter.h"
#include "mocks/mock_history.h"

class StopFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<StopFilter>(3.0);
        history = std::make_unique<MockHistory>();
    }

    GpsPoint createPoint(double lat, double lon, double speed, unsigned long long time = 0, bool valid = true) {
        GpsPoint p;
        p.latitude = lat;
        p.longitude = lon;
        p.speed = speed;
        p.timestamp = time;
        p.isValid = valid;
        return p;
    }

    std::unique_ptr<StopFilter> filter;
    std::unique_ptr<MockHistory> history;
};

TEST_F(StopFilterTest, Process_Enabled_SpeedAboveThreshold_ReturnsPass) {
    auto point = createPoint(48.1173, 11.5167, 5.0);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_EQ(point.speed, 5.0);
}

TEST_F(StopFilterTest, Process_Enabled_SpeedBelowThreshold_NoHistory_ReturnsStop) {
    auto point = createPoint(48.1173, 11.5167, 1.5);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::STOP);
    EXPECT_EQ(point.speed, 1.5);
}

TEST_F(StopFilterTest, Process_Enabled_SpeedBelowThreshold_WithHistory_ReturnsStopAndModifiesPoint) {
    auto lastValid = createPoint(48.1173, 11.5167, 10.0, 1000);
    history->addPoint(lastValid);

    auto point = createPoint(48.1175, 11.5169, 1.5, 2000);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::STOP);
    EXPECT_NEAR(point.latitude, 48.1173, 0.0001);
    EXPECT_NEAR(point.longitude, 11.5167, 0.0001);
    EXPECT_NEAR(point.speed, 0.0, 0.1);
}

TEST_F(StopFilterTest, Process_Enabled_InvalidPoint_ReturnsReject) {
    auto point = createPoint(48.1173, 11.5167, 1.5, 0, false);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(StopFilterTest, Process_Disabled_ReturnsPass) {
    filter->setEnabled(false);

    auto point = createPoint(48.1173, 11.5167, 1.5);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_EQ(point.speed, 1.5);
}

TEST_F(StopFilterTest, SetThreshold_ChangesThreshold) {
    filter->setThreshold(5.0);

    auto point = createPoint(48.1173, 11.5167, 4.0);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::STOP);
}

TEST_F(StopFilterTest, GetThreshold_ReturnsCorrectValue) {
    EXPECT_EQ(filter->getThreshold(), 3.0);

    filter->setThreshold(2.5);
    EXPECT_EQ(filter->getThreshold(), 2.5);
}

TEST_F(StopFilterTest, GetName_ReturnsCorrectName) {
    EXPECT_EQ(filter->getName(), "StopFilter");
}
