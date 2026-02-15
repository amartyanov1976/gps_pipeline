#include <gtest/gtest.h>
#include "pipeline.h"
#include "mock_display.h"
#include "satellite_filter.h"
#include "speed_filter.h"
#include "jump_filter.h"
#include "stop_filter.h"

class PipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto mock = std::make_unique<MockDisplay>();
        mockDisplay = mock.get();
        pipeline = std::make_unique<GpsPipeline>(std::move(mock));
    }
    
    void processPair(const std::string& rmc, const std::string& gga) {
        pipeline->process(rmc);
        pipeline->process(gga);
    }
    
    MockDisplay* mockDisplay;
    std::unique_ptr<GpsPipeline> pipeline;
};

TEST_F(PipelineTest, Process_ValidRMCAndGGA_ShowsPoint) {
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(mockDisplay->getPointCount(), 2);
    EXPECT_EQ(mockDisplay->getInvalidFixCount(), 0);
    EXPECT_EQ(mockDisplay->getErrorCount(), 0);
    
    auto calls = mockDisplay->getCalls();
    ASSERT_FALSE(calls.empty());
    EXPECT_EQ(calls[0].type, DisplayCall::Type::POINT);
    EXPECT_NEAR(calls[0].point.latitude, 48.1173, 0.0001);
}

TEST_F(PipelineTest, Process_InvalidChecksum_ShowsParseError) {
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF");
    
    EXPECT_EQ(mockDisplay->getPointCount(), 0);
    EXPECT_EQ(mockDisplay->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_WithSatelliteFilter_RejectsLowSatellites) {
    auto filter = std::make_unique<SatelliteFilter>(4);
    pipeline->addFilter(std::move(filter), 1);
    
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,02,0.9,545.4,M,47.0,M,,*4E"
    );
    
    EXPECT_EQ(mockDisplay->getPointCount(), 1);
    EXPECT_EQ(mockDisplay->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_WithSpeedFilter_RejectsHighSpeed) {
    auto filter = std::make_unique<SpeedFilter>(50.0);
    pipeline->addFilter(std::move(filter), 1);
    
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,100.0,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(mockDisplay->getPointCount(), 1);
    EXPECT_EQ(mockDisplay->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_WithJumpFilter_RejectsLargeJump) {
    auto filter = std::make_unique<JumpFilter>(10.0);
    pipeline->addFilter(std::move(filter), 1);
    
    // Первая точка
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(mockDisplay->getPointCount(), 2);
    
    // Скачок
    processPair(
        "$GPRMC,123520,A,4808.000,N,01132.000,E,022.4,084.4,230394,,,*30",
        "$GPGGA,123520,4808.000,N,01132.000,E,1,08,0.9,545.4,M,47.0,M,,*42"
    );
    
    EXPECT_EQ(mockDisplay->getPointCount(), 2);
    EXPECT_EQ(mockDisplay->getErrorCount(), 2);
}

TEST_F(PipelineTest, Process_WithStopFilter_ModifiesPoint) {
    auto filter = std::make_unique<StopFilter>(3.0);
    pipeline->addFilter(std::move(filter), 1);
    
    // Первая точка (движение)
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    // Вторая точка (низкая скорость)
    processPair(
        "$GPRMC,123520,A,4807.039,N,01131.001,E,001.5,084.4,230394,,,*37",
        "$GPGGA,123520,4807.039,N,01131.001,E,1,08,0.9,545.4,M,47.0,M,,*45"
    );
    
    auto calls = mockDisplay->getCalls();
    ASSERT_EQ(calls.size(), 4);
    
    // Проверяем, что вторая точка имеет координаты первой и скорость 0
    EXPECT_NEAR(calls[2].point.latitude, 48.1173, 0.0001);
    EXPECT_NEAR(calls[2].point.longitude, 11.5167, 0.0001);
    EXPECT_NEAR(calls[2].point.speed, 0.0, 0.1);
}

TEST_F(PipelineTest, Process_MultipleFilters_RespectsPriority) {
    // Фильтр скорости (приоритет 1) - отклонит
    auto speedFilter = std::make_unique<SpeedFilter>(50.0);
    // Фильтр спутников (приоритет 2)
    auto satFilter = std::make_unique<SatelliteFilter>(4);
    
    pipeline->addFilter(std::move(speedFilter), 1);
    pipeline->addFilter(std::move(satFilter), 2);
    
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,100.0,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    // Должен быть отклонён фильтром скорости (не дойдёт до спутникового)
    EXPECT_EQ(mockDisplay->getPointCount(), 1);
    EXPECT_EQ(mockDisplay->getErrorCount(), 1);
}

TEST_F(PipelineTest, GetStatistics_ReturnsCorrectCounts) {
    EXPECT_EQ(pipeline->getProcessedCount(), 0);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    
    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 1);
    
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 2);
}

TEST_F(PipelineTest, SetHistorySize_ChangesHistorySize) {
    pipeline->setHistorySize(2);
    
    for (int i = 0; i < 5; i++) {
        std::string time = std::to_string(123519 + i);
        processPair(
            "$GPRMC," + time + ",A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
            "$GPGGA," + time + ",4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
        );
    }
    
    EXPECT_EQ(pipeline->getHistory().size(), 2);
}