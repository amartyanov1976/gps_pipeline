#include <gtest/gtest.h>
#include "pipeline.h"
#include "filters/satellite_filter.h"
#include "filters/speed_filter.h"
#include "filters/jump_filter.h"
#include "filters/stop_filter.h"

using namespace gps;

class PipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        pipeline = std::make_unique<GpsPipeline>(5);
        
        validPoint = GpsPoint::zero();
        validPoint.setValid(true);
        validPoint.setLatitude(55.752056);
        validPoint.setLongitude(37.659463);
        validPoint.setSpeed(50.0);
        validPoint.setSatellites(10);
        validPoint.setTimestamp(std::chrono::system_clock::now());
        
        invalidPoint = GpsPoint::invalid();
    }
    
    std::unique_ptr<GpsPipeline> pipeline;
    GpsPoint validPoint;
    GpsPoint invalidPoint;
};

TEST_F(PipelineTest, AddFilters) {
    EXPECT_EQ(pipeline->getFilterCount(), 0);
    
    pipeline->addFilter(std::make_unique<SatelliteFilter>(4));
    pipeline->addFilter(std::make_unique<SpeedFilter>(200.0));
    
    EXPECT_EQ(pipeline->getFilterCount(), 2);
    EXPECT_NE(pipeline->getFilter(0), nullptr);
    EXPECT_NE(pipeline->getFilter(1), nullptr);
}

TEST_F(PipelineTest, ProcessValidPoint) {
    pipeline->addFilter(std::make_unique<SatelliteFilter>(4));
    pipeline->addFilter(std::make_unique<SpeedFilter>(200.0));
    
    auto result = pipeline->process(validPoint);
    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_EQ(pipeline->getHistory().size(), 1);
    EXPECT_EQ(pipeline->getProcessedCount(), 1);
}

TEST_F(PipelineTest, RejectInvalidPoint) {
    auto result = pipeline->process(invalidPoint);
    EXPECT_EQ(result, FilterResult::REJECT);
    EXPECT_EQ(pipeline->getHistory().size(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 1);
}

TEST_F(PipelineTest, FilterPriority) {
    pipeline->addFilter(std::make_unique<SatelliteFilter>(4));
    pipeline->addFilter(std::make_unique<SpeedFilter>(200.0));
    
    validPoint.setSatellites(2); // Должен отклонить первый фильтр
    
    auto result = pipeline->process(validPoint);
    EXPECT_EQ(result, FilterResult::REJECT);
    EXPECT_EQ(pipeline->getHistory().size(), 0);
    
    validPoint.setSatellites(10);
    validPoint.setSpeed(250.0); // Должен отклонить второй фильтр
    
    result = pipeline->process(validPoint);
    EXPECT_EQ(result, FilterResult::REJECT);
}

TEST_F(PipelineTest, EnableDisableFilters) {
    auto filter = std::make_unique<SatelliteFilter>(4);
    auto* filterPtr = filter.get();
    pipeline->addFilter(std::move(filter));
    
    validPoint.setSatellites(2);
    
    // Фильтр включен - отклоняет
    auto result = pipeline->process(validPoint);
    EXPECT_EQ(result, FilterResult::REJECT);
    
    // Отключаем фильтр
    pipeline->enableFilter(0, false);
    result = pipeline->process(validPoint);
    EXPECT_EQ(result, FilterResult::PASS);
    
    // Включаем обратно
    pipeline->enableFilter(0, true);
    EXPECT_TRUE(filterPtr->isEnabled());
}

TEST_F(PipelineTest, HistoryManagement) {
    pipeline->setHistorySize(3);
    
    for (int i = 0; i < 5; i++) {
        validPoint.setTimestamp(validPoint.getTimestamp() + std::chrono::seconds(1));
        pipeline->process(validPoint);
    }
    
    EXPECT_EQ(pipeline->getHistory().size(), 3);
    EXPECT_EQ(pipeline->getHistorySize(), 3);
    
    pipeline->clearHistory();
    EXPECT_EQ(pipeline->getHistory().size(), 0);
}

TEST_F(PipelineTest, RemoveFilter) {
    pipeline->addFilter(std::make_unique<SatelliteFilter>(4));
    pipeline->addFilter(std::make_unique<SpeedFilter>(200.0));
    EXPECT_EQ(pipeline->getFilterCount(), 2);
    
    pipeline->removeFilter(0);
    EXPECT_EQ(pipeline->getFilterCount(), 1);
    EXPECT_NE(pipeline->getFilter(0)->name(), "SatelliteFilter");
}

TEST_F(PipelineTest, ClearFilters) {
    pipeline->addFilter(std::make_unique<SatelliteFilter>(4));
    pipeline->addFilter(std::make_unique<SpeedFilter>(200.0));
    EXPECT_EQ(pipeline->getFilterCount(), 2);
    
    pipeline->clearFilters();
    EXPECT_EQ(pipeline->getFilterCount(), 0);
}

TEST_F(PipelineTest, ResetStats) {
    pipeline->process(invalidPoint); // reject
    pipeline->process(validPoint);   // pass
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getRejectedCount(), 1);
    
    pipeline->resetStats();
    EXPECT_EQ(pipeline->getProcessedCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
}