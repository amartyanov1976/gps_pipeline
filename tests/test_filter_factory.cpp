#include <gtest/gtest.h>
#include "infra/filter_factory.h"
#include "mocks/mock_history.h"

TEST(FilterFactoryTest, Create_SpeedFilter) {
    FilterConfig config;
    config.type = "SpeedFilter";
    config.enabled = true;
    config.params["maxSpeed"] = 50.0;

    auto filter = FilterFactory::create(config);

    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->getName(), "SpeedFilter");
    EXPECT_TRUE(filter->isEnabled());

    MockHistory history;
    GpsPoint point;
    point.speed = 60.0;
    point.isValid = true;
    EXPECT_EQ(filter->process(point, history), FilterResult::REJECT);

    point.speed = 40.0;
    EXPECT_EQ(filter->process(point, history), FilterResult::PASS);
}

TEST(FilterFactoryTest, Create_SatelliteFilter) {
    FilterConfig config;
    config.type = "SatelliteFilter";
    config.enabled = true;
    config.params["minSatellites"] = 6;

    auto filter = FilterFactory::create(config);

    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->getName(), "SatelliteFilter");
}

TEST(FilterFactoryTest, Create_JumpFilter) {
    FilterConfig config;
    config.type = "JumpFilter";
    config.enabled = true;
    config.params["maxJump"] = 200.0;

    auto filter = FilterFactory::create(config);

    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->getName(), "JumpFilter");
}

TEST(FilterFactoryTest, Create_StopFilter) {
    FilterConfig config;
    config.type = "StopFilter";
    config.enabled = true;
    config.params["threshold"] = 5.0;

    auto filter = FilterFactory::create(config);

    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->getName(), "StopFilter");
}

TEST(FilterFactoryTest, Create_SmoothingFilter) {
    FilterConfig config;
    config.type = "SmoothingFilter";
    config.enabled = true;
    config.params["cutoffFrequency"] = 0.2;
    config.params["sampleRate"] = 2.0;

    auto filter = FilterFactory::create(config);

    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->getName(), "SmoothingFilter");
}

TEST(FilterFactoryTest, Create_UnknownType_ReturnsNull) {
    FilterConfig config;
    config.type = "UnknownFilter";

    auto filter = FilterFactory::create(config);

    EXPECT_EQ(filter, nullptr);
}

TEST(FilterFactoryTest, Create_DisabledFilter) {
    FilterConfig config;
    config.type = "SpeedFilter";
    config.enabled = false;

    auto filter = FilterFactory::create(config);

    ASSERT_NE(filter, nullptr);
    EXPECT_FALSE(filter->isEnabled());
}

TEST(FilterFactoryTest, Create_DefaultParams) {
    FilterConfig config;
    config.type = "SpeedFilter";
    config.enabled = true;

    auto filter = FilterFactory::create(config);

    ASSERT_NE(filter, nullptr);

    MockHistory history;
    GpsPoint point;
    point.speed = 250.0;
    point.isValid = true;
    EXPECT_EQ(filter->process(point, history), FilterResult::PASS);

    point.speed = 350.0;
    EXPECT_EQ(filter->process(point, history), FilterResult::REJECT);
}
