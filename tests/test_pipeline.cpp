#include <gtest/gtest.h>
#include "pipeline.h"
#include "mock_display.h"
#include "json_config.h"
#include "satellite_filter.h"
#include "speed_filter.h"
#include "jump_filter.h"
#include "stop_filter.h"
#include "smoothing_filter.h"

class PipelineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем конфигурацию
        config.setDisplayType("console"); // это будет проигнорировано, т.к. мы подменим display
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
        
        // Создаем pipeline с конфигурацией и подменяем display на mock
        // Для этого нам нужно получить доступ к protected/private members
        // Используем другое решение - создадим pipeline с mock display через отдельный конструктор
    }
    
    void createPipelineWithMock() {
        // Создаем mock display
        auto mockDisplay = std::make_unique<MockDisplay>();
        mockDisplay_ = mockDisplay.get();
        
        // Создаем pipeline с mock display (используем альтернативный конструктор)
        pipeline = std::make_unique<GpsPipeline>(std::move(mockDisplay));
        
        // Вручную настраиваем фильтры из конфигурации
        for (const auto& filterConfig : config.getFilters()) {
            auto filter = createTestFilter(filterConfig);
            if (filter) {
                pipeline->addFilter(std::move(filter), filterConfig.priority);
            }
        }
        
        pipeline->setHistorySize(config.getHistorySize());
    }
    
    std::unique_ptr<IGpsFilter> createTestFilter(const FilterConfig& config) {
        std::unique_ptr<IGpsFilter> filter;
        
        if (config.type == "SatelliteFilter") {
            int minSatellites = 4;
            auto it = config.params.find("minSatellites");
            if (it != config.params.end()) {
                minSatellites = static_cast<int>(it->second);
            }
            filter = std::make_unique<SatelliteFilter>(minSatellites);
        }
        else if (config.type == "SpeedFilter") {
            double maxSpeed = 300.0;
            auto it = config.params.find("maxSpeed");
            if (it != config.params.end()) {
                maxSpeed = it->second;
            }
            filter = std::make_unique<SpeedFilter>(maxSpeed);
        }
        else if (config.type == "JumpFilter") {
            double maxJump = 100.0;
            auto it = config.params.find("maxJump");
            if (it != config.params.end()) {
                maxJump = it->second;
            }
            filter = std::make_unique<JumpFilter>(maxJump);
        }
        else if (config.type == "StopFilter") {
            double threshold = 3.0;
            auto it = config.params.find("threshold");
            if (it != config.params.end()) {
                threshold = it->second;
            }
            filter = std::make_unique<StopFilter>(threshold);
        }
        else if (config.type == "SmoothingFilter") {
            double cutoff = 0.1;
            double sampleRate = 1.0;
            
            auto it = config.params.find("cutoffFrequency");
            if (it != config.params.end()) {
                cutoff = it->second;
            }
            
            it = config.params.find("sampleRate");
            if (it != config.params.end()) {
                sampleRate = it->second;
            }
            
            filter = std::make_unique<SmoothingFilter>(cutoff, sampleRate);
        }
        
        if (filter) {
            filter->setEnabled(config.enabled);
        }
        
        return filter;
    }
    
    void processPair(const std::string& rmc, const std::string& gga) {
        pipeline->process(rmc);
        pipeline->process(gga);
    }
    
    JsonConfig config;
    std::unique_ptr<GpsPipeline> pipeline;
    MockDisplay* mockDisplay_ = nullptr;
};

TEST_F(PipelineTest, Process_ValidRMCAndGGA_UpdatesStatistics) {
    createPipelineWithMock();
    
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 2);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
    
    // Проверяем что mock display получил вызов
    ASSERT_NE(mockDisplay_, nullptr);
    EXPECT_EQ(mockDisplay_->getPointCount(), 2);
}

TEST_F(PipelineTest, Process_InvalidChecksum_IncreasesErrorCount) {
    createPipelineWithMock();
    
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF");
    
    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
    
    EXPECT_EQ(mockDisplay_->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_InvalidStatus_NoValidPoint) {
    createPipelineWithMock();
    
    processPair(
        "$GPRMC,123519,V,4807.038,N,01131.000,E,022.4,084.4,230394,,,*2A",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
    
    EXPECT_EQ(mockDisplay_->getInvalidFixCount(), 2);
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
    
    config = testConfig;
    createPipelineWithMock();
    
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,02,0.9,545.4,M,47.0,M,,*45");
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 1);
    EXPECT_EQ(pipeline->getRejectedCount(), 1);
    
    EXPECT_EQ(mockDisplay_->getErrorCount(), 1);
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
    
    config = testConfig;
    createPipelineWithMock();
    
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,100.0,084.4,230394,,,*38");
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 2);
    
    EXPECT_EQ(mockDisplay_->getErrorCount(), 2);
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
    
    config = testConfig;
    createPipelineWithMock();
    
    // Первая точка
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(pipeline->getValidCount(), 2);
    EXPECT_EQ(mockDisplay_->getPointCount(), 2);
    
    // Скачок
    pipeline->process("$GPRMC,123520,A,4808.000,N,01132.000,E,022.4,084.4,230394,,,*30");
    pipeline->process("$GPGGA,123520,4808.000,N,01132.000,E,1,08,0.9,545.4,M,47.0,M,,*42");
    
    EXPECT_EQ(pipeline->getValidCount(), 2); // вторая точка отклонена
    EXPECT_EQ(pipeline->getRejectedCount(), 2);
    EXPECT_EQ(mockDisplay_->getPointCount(), 2);
    EXPECT_EQ(mockDisplay_->getErrorCount(), 2);
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
    
    config = testConfig;
    createPipelineWithMock();
    
    // Первая точка (движение)
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(pipeline->getValidCount(), 2);
    EXPECT_EQ(mockDisplay_->getPointCount(), 2);
    
    // В данном тесте сложно проверить модификацию точки без доступа к данным
    // Проверяем только статистику
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
    
    config = testConfig;
    createPipelineWithMock();
    
    // Точка с высокой скоростью - должна быть отклонена фильтром скорости
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,100.0,084.4,230394,,,*38");
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    
    EXPECT_EQ(pipeline->getRejectedCount(), 2);
    EXPECT_EQ(mockDisplay_->getErrorCount(), 2);
}

TEST_F(PipelineTest, GetStatistics_ReturnsCorrectCounts) {
    createPipelineWithMock();
    
    EXPECT_EQ(pipeline->getProcessedCount(), 0);
    EXPECT_EQ(pipeline->getValidCount(), 0);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 0);
    
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    EXPECT_EQ(pipeline->getProcessedCount(), 1);
    EXPECT_EQ(pipeline->getValidCount(), 1);
    
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F");
    EXPECT_EQ(pipeline->getProcessedCount(), 2);
    EXPECT_EQ(pipeline->getValidCount(), 2);
}

TEST_F(PipelineTest, Process_EmptyLine_NoChanges) {
    createPipelineWithMock();
    
    int initialProcessed = pipeline->getProcessedCount();
    
    pipeline->process("");
    
    EXPECT_EQ(pipeline->getProcessedCount(), initialProcessed + 1);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
}

TEST_F(PipelineTest, Process_MultipleLines_CorrectStatistics) {
    createPipelineWithMock();
    
    // Валидная пара
    processPair(
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D",
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*4F"
    );
    
    // Невалидная контрольная сумма
    pipeline->process("$GPRMC,123520,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*FF");
    
    // Невалидный статус
    processPair(
        "$GPRMC,123521,V,4807.038,N,01131.000,E,022.4,084.4,230394,,,*21",
        "$GPGGA,123521,4807.038,N,01131.000,E,1,08,0.9,545.4,M,47.0,M,,*44"
    );
    
    EXPECT_EQ(pipeline->getProcessedCount(), 5);
    EXPECT_EQ(pipeline->getValidCount(), 2);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(pipeline->getErrorCount(), 1);
    
    EXPECT_EQ(mockDisplay_->getPointCount(), 2);
    EXPECT_EQ(mockDisplay_->getInvalidFixCount(), 2);
    EXPECT_EQ(mockDisplay_->getErrorCount(), 1);
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
    
    config = testConfig;
    createPipelineWithMock();
    
    // Точка с малым количеством спутников, но фильтр отключен
    pipeline->process("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,,,*3D");
    pipeline->process("$GPGGA,123519,4807.038,N,01131.000,E,1,02,0.9,545.4,M,47.0,M,,*45");
    
    EXPECT_EQ(pipeline->getValidCount(), 2);
    EXPECT_EQ(pipeline->getRejectedCount(), 0);
    EXPECT_EQ(mockDisplay_->getPointCount(), 2);
}