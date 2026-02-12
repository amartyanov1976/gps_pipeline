#include <gtest/gtest.h>
#include "filters/stop_filter.h"
#include "gps_point.h"
#include <chrono>

using namespace gps;

class StopFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<StopFilter>(3.0, 5);
        
        movingPoint = GpsPoint::zero();
        movingPoint.setValid(true);
        movingPoint.setLatitude(55.752056);
        movingPoint.setLongitude(37.659463);
        movingPoint.setSpeed(50.0);
        movingPoint.setTimestamp(std::chrono::system_clock::now());
        
        stoppingPoint = movingPoint;
        stoppingPoint.setSpeed(1.5);
        
        history.push_back(movingPoint);
    }
    
    std::unique_ptr<StopFilter> filter;
    GpsPoint movingPoint;
    GpsPoint stoppingPoint;
    std::deque<GpsPoint> history;
};

TEST_F(StopFilterTest, PassMoving) {
    auto result = filter->process(movingPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(StopFilterTest, StartStop) {
    auto result = filter->process(stoppingPoint, history);
    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_NEAR(stoppingPoint.getSpeed(), 1.5, 0.1);
}

TEST_F(StopFilterTest, StopAfterTime) {
    // Первая точка - начало остановки
    filter->process(stoppingPoint, history);
    
    // Вторая точка через 6 секунд
    stoppingPoint.setTimestamp(stoppingPoint.getTimestamp() + std::chrono::seconds(6));
    auto result = filter->process(stoppingPoint, history);
    
    EXPECT_EQ(result, FilterResult::STOP);
    EXPECT_NEAR(stoppingPoint.getSpeed(), 0.0, 0.1);
    EXPECT_NEAR(stoppingPoint.getLatitude(), movingPoint.getLatitude(), 0.0001);
}

TEST_F(StopFilterTest, ResumeMovement) {
    // Начало остановки
    filter->process(stoppingPoint, history);
    
    // Возобновление движения
    movingPoint.setSpeed(10.0);
    movingPoint.setTimestamp(movingPoint.getTimestamp() + std::chrono::seconds(2));
    auto result = filter->process(movingPoint, history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_FALSE(filter->isStopped());
}

TEST_F(StopFilterTest, Reset) {
    // Начало остановки
    filter->process(stoppingPoint, history);
    EXPECT_TRUE(filter->isStopped());
    
    // Сброс
    filter->reset();
    EXPECT_FALSE(filter->isStopped());
}

TEST_F(StopFilterTest, Configure) {
    filter->setThreshold(5.0);
    EXPECT_EQ(filter->getThreshold(), 5.0);
    
    filter->setMinStopTime(10);
    EXPECT_EQ(filter->getMinStopTime(), 10);
}

TEST_F(StopFilterTest, Name) {
    EXPECT_EQ(filter->name(), "StopFilter");
}