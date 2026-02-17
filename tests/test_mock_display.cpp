#include <gtest/gtest.h>
#include "mocks/mock_display.h"

class MockDisplayTest : public ::testing::Test {
protected:
    void SetUp() override {
        display = std::make_unique<MockDisplay>();
    }

    GpsPoint createValidPoint(unsigned long long time = 123519000) {
        GpsPoint p;
        p.latitude = 48.1173;
        p.longitude = 11.5167;
        p.speed = 41.5;
        p.course = 84.4;
        p.altitude = 545.4;
        p.satellites = 8;
        p.hdop = 0.9f;
        p.timestamp = time;
        p.isValid = true;
        return p;
    }

    std::unique_ptr<MockDisplay> display;
};

TEST_F(MockDisplayTest, ShowPoint_AddsCall) {
    auto point = createValidPoint();

    display->showPoint(point);

    auto calls = display->getCalls();
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls[0].type, DisplayCall::Type::POINT);
    EXPECT_EQ(calls[0].timestamp, point.timestamp);
    EXPECT_EQ(calls[0].point.latitude, point.latitude);
}

TEST_F(MockDisplayTest, ShowInvalidFix_AddsCall) {
    display->showInvalidFix(123519000);

    auto calls = display->getCalls();
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls[0].type, DisplayCall::Type::INVALID_FIX);
    EXPECT_EQ(calls[0].timestamp, 123519000);
}

TEST_F(MockDisplayTest, ShowParseError_AddsCall) {
    display->showParseError("test error");

    auto calls = display->getCalls();
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls[0].type, DisplayCall::Type::PARSE_ERROR);
    EXPECT_EQ(calls[0].message, "test error");
}

TEST_F(MockDisplayTest, ShowRejected_AddsCall) {
    display->showRejected("test rejection");

    auto calls = display->getCalls();
    ASSERT_EQ(calls.size(), 1);
    EXPECT_EQ(calls[0].type, DisplayCall::Type::REJECTED);
    EXPECT_EQ(calls[0].message, "test rejection");
}

TEST_F(MockDisplayTest, Clear_RemovesAllCalls) {
    display->showPoint(createValidPoint());
    display->showInvalidFix(123519000);

    EXPECT_EQ(display->getCalls().size(), 2);

    display->clear();

    EXPECT_EQ(display->getCalls().size(), 0);
}

TEST_F(MockDisplayTest, Reset_ClearsCalls) {
    display->showPoint(createValidPoint());

    EXPECT_EQ(display->getCalls().size(), 1);

    display->reset();

    EXPECT_EQ(display->getCalls().size(), 0);
}

TEST_F(MockDisplayTest, HasPointWithTime_ReturnsTrueWhenExists) {
    display->showPoint(createValidPoint(123519000));
    display->showPoint(createValidPoint(123520000));

    EXPECT_TRUE(display->hasPointWithTime(123519000));
    EXPECT_TRUE(display->hasPointWithTime(123520000));
    EXPECT_FALSE(display->hasPointWithTime(123521000));
}

TEST_F(MockDisplayTest, GetPointCount_ReturnsCorrectCount) {
    display->showPoint(createValidPoint());
    display->showPoint(createValidPoint());
    display->showInvalidFix(123519000);

    EXPECT_EQ(display->getPointCount(), 2);
}

TEST_F(MockDisplayTest, GetInvalidFixCount_ReturnsCorrectCount) {
    display->showInvalidFix(123519000);
    display->showInvalidFix(123520000);
    display->showPoint(createValidPoint());

    EXPECT_EQ(display->getInvalidFixCount(), 2);
}

TEST_F(MockDisplayTest, GetErrorCount_ReturnsCorrectCount) {
    display->showParseError("error1");
    display->showRejected("rejection1");
    display->showPoint(createValidPoint());

    EXPECT_EQ(display->getErrorCount(), 2);
}
