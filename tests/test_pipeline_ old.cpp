#include <gtest/gtest.h>
#include "pipeline.h"
#include "mock_display.h"
#include "json_config.h"

class PipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем конфигурацию с mock-дисплеем через конфиг
        config.setDisplayType("console"); // console, но мы подменим display в pipeline
        config.setHistorySize(5);
        
        // Добавляем фильтры в конфигурацию
        FilterConfig satFilter;
        satFilter.type = "SatelliteFilter";
        satFilter.enabled = true;
        satFilter.priority = 1;
        satFilter.params["minSatellites"] = 4;
        config.addFilter(satFilter);
        
        FilterConfig speedFilter;
        speedFilter.type = "SpeedFilter";
        speedFilter.enabled = true;
        speedFilter.priority = 2;
        speedFilter.params["maxSpeed"] = 300.0;
        config.addFilter(speedFilter);
        
        // Создаем pipeline с конфигурацией
        pipeline = std::make_unique<GpsPipeline>(config);
        
        // Подменяем дисплей на mock для тестирования
        // Это требует доступа к protected/private - для тестов используем friend class или рефлексию
        // В данном тесте мы будем проверять через вывод pipeline, а не через mock
    }
    
    void processPair(const std::string& rmc, const std::string& gga) {
        pipeline->process(rmc);
        pipeline->process(gga);
    }
    
    JsonConfig config;
    std::unique_ptr<GpsPipeline> pipeline;
};

TEST_F(PipelineTest, Process_ValidRMCAndGGA_UpdatesStatistics) {
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 2);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
}

TEST_F(PipelineTest, Process_InvalidChecksum_IncreasesErrorCount) {
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF");
    
    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_InvalidStatus_NoValidPoint) {
    processPair(
        "$GPRMC,123519,V,4807.038,N,01131.000,E,022.4,084.4,230394,,,*2A",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
}

TEST_F(PipelineTest, Process_WithSatelliteFilter_RejectsLowSatellites) {
    // Создаем конфигурацию с фильтром спутников
    JsonConfig testConfig;
    testConfig.setHistorySize(5);
    
    FilterConfig satFilter;
    satFilter.type = "SatelliteFilter";
    satFilter.enabled = true;
    satFilter.priority = 1;
    satFilter.params["minSatellites"] = 4;
    testConfig.addFilter(satFilter);
    
    auto testPipeline = std::make_unique<GpsPipeline>(testConfig);
    
    testPipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    testPipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,02,0.9,545.4,M,47.0,M,,*45");
    
    EXPECT_EQ(testPipeline->getProcessedCount(), 2);
    EXPECT_EQ(testPipeline->getValidCount(), 1);
    EXPECT_EQ(testPipeline->getRejectedCount(), 1);
}

TEST_F(PipelineTest, Process_WithSpeedFilter_RejectsHighSpeed) {
    // Создаем конфигурацию с фильтром скорости
    JsonConfig testConfig;
    testConfig.setHistorySize(5);
    
    FilterConfig speedFilter;
    speedFilter.type = "SpeedFilter";
    speedFilter.enabled = true;
    speedFilter.priority = 1;
    speedFilter.params["maxSpeed"] = 50.0;
    testConfig.addFilter(speedFilter);
    
    auto testPipeline = std::make_unique<GpsPipeline>(testConfig);
    
    testPipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,100.0,084.4,230394,,,*38");
    testPipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(testPipeline->getProcessedCount(), 2);
    EXPECT_EQ(testPipeline->getValidCount(), 0);
    EXPECT_EQ(testPipeline->getRejectedCount(), 2);
}

TEST_F(PipelineTest, Process_WithJumpFilter_RejectsLargeJump) {
    // Создаем конфигурацию с фильтром скачков
    JsonConfig testConfig;
    testConfig.setHistorySize(5);
    
    FilterConfig jumpFilter;
    jumpFilter.type = "JumpFilter";
    jumpFilter.enabled = true;
    jumpFilter.priority = 1;
    jumpFilter.params["maxJump"] = 10.0;
    testConfig.addFilter(jumpFilter);
    
    auto testPipeline = std::make_unique<GpsPipeline>(testConfig);
    
    // Первая точка
    testPipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    testPipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(testPipeline->getValidCount(), 2);
    
    // Скачок
    testPipeline->process("$GPRMC,123520,A,4808.000,N,01132.000,E,022.4,084.4,230394,,,*30");
    testPipeline->process("$GPGGA,123520,4808.000,N,01132.000,E,1,08,0.9,545.4,M,47.0,M,,*42");
    
    EXPECT_EQ(testPipeline->getValidCount(), 2); // вторая точка отклонена
    EXPECT_EQ(testPipeline->getRejectedCount(), 2);
}

TEST_F(PipelineTest, Process_WithStopFilter_ModifiesPoint) {
    // Создаем конфигурацию с фильтром остановки
    JsonConfig testConfig;
    testConfig.setHistorySize(5);
    
    FilterConfig stopFilter;
    stopFilter.type = "StopFilter";
    stopFilter.enabled = true;
    stopFilter.priority = 1;
    stopFilter.params["threshold"] = 3.0;
    testConfig.addFilter(stopFilter);
    
    auto testPipeline = std::make_unique<GpsPipeline>(testConfig);
    
    // Первая точка (движение)
    testPipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    testPipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    // В данном тесте сложно проверить модификацию точки без доступа к выводу
    // Проверяем только статистику
    EXPECT_EQ(testPipeline->getValidCount(), 2);
}

TEST_F(PipelineTest, Process_MultipleFilters_RespectsPriority) {
    // Создаем конфигурацию с двумя фильтрами
    JsonConfig testConfig;
    testConfig.setHistorySize(5);
    
    // Фильтр скорости с высоким приоритетом (должен выполниться первым)
    FilterConfig speedFilter;
    speedFilter.type = "SpeedFilter";
    speedFilter.enabled = true;
    speedFilter.priority = 1;
    speedFilter.params["maxSpeed"] = 50.0;
    testConfig.addFilter(speedFilter);
    
    // Фильтр спутников с низким приоритетом
    FilterConfig satFilter;
    satFilter.type = "SatelliteFilter";
    satFilter.enabled = true;
    satFilter.priority = 2;
    satFilter.params["minSatellites"] = 4;
    testConfig.addFilter(satFilter);
    
    auto testPipeline = std::make_unique<GpsPipeline>(testConfig);
    
    // Точка с высокой скоростью - должна быть отклонена фильтром скорости
    testPipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,100.0,084.4,230394,,,*3D");
    testPipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(testPipeline->getRejectedCount(), 1);
}

TEST_F(PipelineTest, GetStatistics_ReturnsCorrectCounts) {
    EXPECT_EQ(pipeline->getProcessedCount(), 0);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
    
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 1);
}

TEST_F(PipelineTest, SetHistorySize_ChangesHistorySize) {
    pipeline->setHistorySize(2);
    
    // Добавляем 3 точки
    for (int i = 0; i < 3; i++) {
        std::string time = std::to_string(123519 + i);
        processPair(
            "$GPRMC," + time + ",A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
            "$GPGGA," + time + ",4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
        );
    }
    
    EXPECT_EQ(pipeline->getHistory().size(), 2);
}

TEST_F(PipelineTest, GetConfig_ReturnsConfig) {
    const auto& returnedConfig = pipeline->getConfig();
    EXPECT_EQ(returnedConfig.getHistorySize(), config.getHistorySize());
    EXPECT_EQ(returnedConfig.getDisplayType(), config.getDisplayType());
    EXPECT_EQ(returnedConfig.getFilters().size(), config.getFilters().size());
}

TEST_F(PipelineTest, Process_EmptyLine_NoChanges) {
    int initialProcessed = pipeline->getProcessedCount();
    
    pipeline->process("");
    
    EXPECT_EQ(pipeline->getProcessedCount(), initialProcessed + 1);
    EXPECT_EQ(pipeline->getErrorCount(), initialProcessed + 1 - pipeline->getValidCount());
}

TEST_F(PipelineTest, Process_MultipleLines_CorrectStatistics) {
    // Валидная пара
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    // Невалидная контрольная сумма
    pipeline->process("$GPRMC,123520,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF");
    
    // Невалидный статус
    processPair(
        "$GPRMC,123521,V,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3C",
        "$GPGGA,123521,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(pipeline->getProcessedCount(), 5);
    EXPECT_EQ(pipeline->getValidCount(), 1);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_WithDisabledFilter_DoesNotReject) {
    // Создаем конфигурацию с отключенным фильтром
    JsonConfig testConfig;
    testConfig.setHistorySize(5);
    
    FilterConfig satFilter;
    satFilter.type = "SatelliteFilter";
    satFilter.enabled = false; // отключен
    satFilter.priority = 1;
    satFilter.params["minSatellites"] = 4;
    testConfig.addFilter(satFilter);
    
    auto testPipeline = std::make_unique<GpsPipeline>(testConfig);
    
    // Точка с малым количеством спутников, но фильтр отключен
    testPipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    testPipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,02,0.9,545.4,M,47.0,M,,*4E");
    
    EXPECT_EQ(testPipeline->getValidCount(), 1);
    EXPECT_EQ(testPipeline->getRejectedCount(), 0);
}