#include <gtest/gtest.h>
#include "history.h"
#include "gps_point.h"

class HistoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        history = std::make_unique<GpsHistory>(3);
    }
    
    GpsPoint createValidPoint(double lat, double lon, unsigned long long time) {
        GpsPoint p;
        p.latitude = lat;
        p.longitude = lon;
        p.timestamp = time;
        p.isValid = true;
        return p;
    }
    
    GpsPoint createInvalidPoint(unsigned long long time) {
        GpsPoint p;
        p.timestamp = time;
        p.isValid = false;
        return p;
    }
    
    std::unique_ptr<GpsHistory> history;
};

TEST_F(HistoryTest, NewHistory_IsEmpty) {
    EXPECT_TRUE(history->empty());
    EXPECT_EQ(history->size(), 0);
}

TEST_F(HistoryTest, AddPoint_IncreasesSize) {
    history->addPoint(createValidPoint(48.1173, 11.5167, 123519000));
    
    EXPECT_FALSE(history->empty());
    EXPECT_EQ(history->size(), 1);
}

TEST_F(HistoryTest, AddPoint_RespectsMaxSize) {
    history->addPoint(createValidPoint(48.1173, 11.5167, 123519000));
    history->addPoint(createValidPoint(48.1174, 11.5168, 123520000));
    history->addPoint(createValidPoint(48.1175, 11.5169, 123521000));
    history->addPoint(createValidPoint(48.1176, 11.5170, 123522000));
    
    EXPECT_EQ(history->size(), 3);
}

TEST_F(HistoryTest, GetLastValid_ReturnsMostRecentValidPoint) {
    history->addPoint(createValidPoint(48.1173, 11.5167, 123519000));
    history->addPoint(createInvalidPoint(123520000));
    history->addPoint(createValidPoint(48.1175, 11.5169, 123521000));
    
    auto last = history->getLastValid();
    ASSERT_TRUE(last.has_value());
    EXPECT_NEAR(last->latitude, 48.1175, 0.0001);
    EXPECT_EQ(last->timestamp, 123521000);
}

TEST_F(HistoryTest, GetLastValid_NoValidPoints_ReturnsNullopt) {
    history->addPoint(createInvalidPoint(123519000));
    history->addPoint(createInvalidPoint(123520000));
    
    auto last = history->getLastValid();
    EXPECT_FALSE(last.has_value());
}

TEST_F(HistoryTest, GetLastValid_EmptyHistory_ReturnsNullopt) {
    auto last = history->getLastValid();
    EXPECT_FALSE(last.has_value());
}

TEST_F(HistoryTest, GetAllPoints_ReturnsAllPoints) {
    auto p1 = createValidPoint(48.1173, 11.5167, 123519000);
    auto p2 = createValidPoint(48.1174, 11.5168, 123520000);
    
    history->addPoint(p1);
    history->addPoint(p2);
    
    auto points = history->getAllPoints();
    EXPECT_EQ(points.size(), 2);
    EXPECT_EQ(points[0].timestamp, 123519000);
    EXPECT_EQ(points[1].timestamp, 123520000);
}

TEST_F(HistoryTest, Clear_RemovesAllPoints) {
    history->addPoint(createValidPoint(48.1173, 11.5167, 123519000));
    history->addPoint(createValidPoint(48.1174, 11.5168, 123520000));
    
    history->clear();
    
    EXPECT_TRUE(history->empty());
    EXPECT_EQ(history->size(), 0);
}