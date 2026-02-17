#include <gtest/gtest.h>
#include "infra/jump_filter.h"
#include "mocks/mock_history.h"

class JumpFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<JumpFilter>(100.0);
        history = std::make_unique<MockHistory>();
    }

    GpsPoint createPoint(double lat, double lon, bool valid = true) {
        GpsPoint p;
        p.latitude = lat;
        p.longitude = lon;
        p.isValid = valid;
        return p;
    }

    std::unique_ptr<JumpFilter> filter;
    std::unique_ptr<MockHistory> history;
};

TEST_F(JumpFilterTest, Process_Enabled_NoHistory_ReturnsPass) {
    auto point = createPoint(48.1173, 11.5167);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(JumpFilterTest, Process_Enabled_SmallJump_ReturnsPass) {
    history->addPoint(createPoint(48.1173, 11.5167));

    auto point = createPoint(48.1174, 11.5168);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(JumpFilterTest, Process_Enabled_LargeJump_ReturnsReject) {
    history->addPoint(createPoint(48.1173, 11.5167));

    auto point = createPoint(48.1200, 11.5200);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(JumpFilterTest, Process_Enabled_InvalidPoint_ReturnsReject) {
    history->addPoint(createPoint(48.1173, 11.5167));

    auto point = createPoint(48.1174, 11.5168, false);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(JumpFilterTest, Process_Disabled_ReturnsPass) {
    filter->setEnabled(false);
    history->addPoint(createPoint(48.1173, 11.5167));

    auto point = createPoint(48.1200, 11.5200);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(JumpFilterTest, SetMaxJump_ChangesThreshold) {
    filter->setMaxJump(500.0);
    history->addPoint(createPoint(48.1173, 11.5167));

    auto point = createPoint(48.1200, 11.5200);
    auto result = filter->process(point, *history);

    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(JumpFilterTest, GetMaxJump_ReturnsCorrectValue) {
    EXPECT_EQ(filter->getMaxJump(), 100.0);

    filter->setMaxJump(250.0);
    EXPECT_EQ(filter->getMaxJump(), 250.0);
}

TEST_F(JumpFilterTest, GetName_ReturnsCorrectName) {
    EXPECT_EQ(filter->getName(), "JumpFilter");
}
