#include <gtest/gtest.h>
#include "smoothing_filter.h"
#include "history.h"

class SmoothingFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<SmoothingFilter>(0.1, 1.0); // cutoff 0.1 Hz, sample rate 1 Hz
        history = std::make_unique<GpsHistory>();
    }
    
    GpsPoint createPoint(double lat, double lon, double speed = 10.0, double alt = 100.0, bool valid = true) {
        GpsPoint p;
        p.latitude = lat;
        p.longitude = lon;
        p.speed = speed;
        p.altitude = alt;
        p.isValid = valid;
        return p;
    }
    
    std::unique_ptr<SmoothingFilter> filter;
    std::unique_ptr<GpsHistory> history;
};

TEST_F(SmoothingFilterTest, Process_Enabled_ValidPoint_ReturnsPass) {
    auto point = createPoint(48.1173, 11.5167);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SmoothingFilterTest, Process_Enabled_InvalidPoint_ReturnsPassWithoutSmoothing) {
    auto point = createPoint(48.1173, 11.5167, 10.0, 100.0, false);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    // Точка не должна измениться
    EXPECT_EQ(point.latitude, 48.1173);
    EXPECT_EQ(point.longitude, 11.5167);
}

TEST_F(SmoothingFilterTest, Process_Disabled_ReturnsPass) {
    filter->setEnabled(false);
    auto point = createPoint(48.1173, 11.5167);
    auto result = filter->process(point, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
}

TEST_F(SmoothingFilterTest, Process_FirstPoint_NoSmoothing) {
    auto point1 = createPoint(48.1173, 11.5167);
    auto result1 = filter->process(point1, *history);
    
    EXPECT_EQ(result1, FilterResult::PASS);
    
    // Создаем историю с первой точкой
    history->addPoint(point1);
    
    auto point2 = createPoint(48.1174, 11.5168);
    auto result2 = filter->process(point2, *history);
    
    EXPECT_EQ(result2, FilterResult::PASS);
    // Вторая точка должна быть сглажена
    EXPECT_NE(point2.latitude, 48.1174);
    EXPECT_NE(point2.longitude, 11.5168);
}

TEST_F(SmoothingFilterTest, Process_SmoothingEffect) {
    // Добавляем первую точку в историю через process
    auto point1 = createPoint(48.1173, 11.5167);
    filter->process(point1, *history);
    history->addPoint(point1);
    
    // Вторая точка - небольшое изменение
    auto point2 = createPoint(48.1175, 11.5169);
    double originalLat = point2.latitude;
    double originalLon = point2.longitude;
    
    auto result = filter->process(point2, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    // Сглаженное значение должно быть между первой и второй точкой
    EXPECT_GT(point2.latitude, point1.latitude);
    EXPECT_LT(point2.latitude, originalLat);
    EXPECT_GT(point2.longitude, point1.longitude);
    EXPECT_LT(point2.longitude, originalLon);
}

TEST_F(SmoothingFilterTest, Process_SpeedAndAltitudeSmoothing) {
    auto point1 = createPoint(48.1173, 11.5167, 10.0, 100.0);
    filter->process(point1, *history);
    history->addPoint(point1);
    
    auto point2 = createPoint(48.1174, 11.5168, 20.0, 120.0);
    double originalSpeed = point2.speed;
    double originalAlt = point2.altitude;
    
    auto result = filter->process(point2, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_GT(point2.speed, point1.speed);
    EXPECT_LT(point2.speed, originalSpeed);
    EXPECT_GT(point2.altitude, point1.altitude);
    EXPECT_LT(point2.altitude, originalAlt);
}

TEST_F(SmoothingFilterTest, SetCutoffFrequency_ChangesSmoothing) {
    filter->setCutoffFrequency(0.5); // Выше частота среза = меньше сглаживание
    
    auto point1 = createPoint(48.1173, 11.5167);
    filter->process(point1, *history);
    history->addPoint(point1);
    
    auto point2 = createPoint(48.1175, 11.5169);
    double originalLat = point2.latitude;
    
    auto result = filter->process(point2, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    // С более высокой частотой среза, сглаживание меньше
    EXPECT_GT(point2.latitude, point1.latitude + (originalLat - point1.latitude) * 0.3);
}

TEST_F(SmoothingFilterTest, GetCutoffFrequency_ReturnsCorrectValue) {
    EXPECT_DOUBLE_EQ(filter->getCutoffFrequency(), 0.1);
    
    filter->setCutoffFrequency(0.3);
    EXPECT_DOUBLE_EQ(filter->getCutoffFrequency(), 0.3);
}

TEST_F(SmoothingFilterTest, SetSampleRate_ChangesSmoothing) {
    filter->setSampleRate(2.0); // Выше частота дискретизации = больше сглаживание
    
    auto point1 = createPoint(48.1173, 11.5167);
    filter->process(point1, *history);
    history->addPoint(point1);
    
    auto point2 = createPoint(48.1175, 11.5169);
    double originalLat = point2.latitude;
    
    auto result = filter->process(point2, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    // С более высокой частотой дискретизации, сглаживание больше
    EXPECT_LT(point2.latitude, point1.latitude + (originalLat - point1.latitude) * 0.7);
}

TEST_F(SmoothingFilterTest, GetSampleRate_ReturnsCorrectValue) {
    EXPECT_DOUBLE_EQ(filter->getSampleRate(), 1.0);
    
    filter->setSampleRate(5.0);
    EXPECT_DOUBLE_EQ(filter->getSampleRate(), 5.0);
}

TEST_F(SmoothingFilterTest, Process_MultiplePoints_ProgressiveSmoothing) {
    std::vector<GpsPoint> points;
    points.push_back(createPoint(48.1173, 11.5167, 10.0, 100.0));
    
    for (int i = 0; i < 5; i++) {
        GpsPoint point = createPoint(
            48.1173 + i * 0.001, 
            11.5167 + i * 0.001,
            10.0 + i * 5.0,
            100.0 + i * 10.0
        );
        
        filter->process(point, *history);
        history->addPoint(point);
        points.push_back(point);
    }
    
    // Проверяем, что последняя точка сглажена, но все еще близка к тренду
    auto lastPoint = points.back();
    EXPECT_NEAR(lastPoint.latitude, 48.1173 + 4 * 0.001, 0.001);
    EXPECT_NEAR(lastPoint.longitude, 11.5167 + 4 * 0.001, 0.001);
}

TEST_F(SmoothingFilterTest, Process_ZeroSpeed_KeepsZero) {
    auto point1 = createPoint(48.1173, 11.5167, 0.0, 100.0);
    filter->process(point1, *history);
    history->addPoint(point1);
    
    auto point2 = createPoint(48.1174, 11.5168, 0.0, 100.0);
    auto result = filter->process(point2, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_DOUBLE_EQ(point2.speed, 0.0);
}

TEST_F(SmoothingFilterTest, Process_NegativeValues_HandledCorrectly) {
    auto point1 = createPoint(-48.1173, -11.5167, 10.0, 100.0);
    filter->process(point1, *history);
    history->addPoint(point1);
    
    auto point2 = createPoint(-48.1175, -11.5169, 15.0, 110.0);
    double originalLat = point2.latitude;
    double originalLon = point2.longitude;
    
    auto result = filter->process(point2, *history);
    
    EXPECT_EQ(result, FilterResult::PASS);
    EXPECT_GT(point2.latitude, originalLat); // -48.1175 > -48.1175? Actually -48.1175 < -48.1173
    EXPECT_LT(point2.latitude, point1.latitude);
    EXPECT_GT(point2.longitude, originalLon);
    EXPECT_LT(point2.longitude, point1.longitude);
}

TEST_F(SmoothingFilterTest, GetName_ReturnsCorrectName) {
    EXPECT_EQ(filter->getName(), "SmoothingFilter");
}

TEST_F(SmoothingFilterTest, IsEnabled_ReturnsCorrectState) {
    EXPECT_TRUE(filter->isEnabled());
    
    filter->setEnabled(false);
    EXPECT_FALSE(filter->isEnabled());
    
    filter->setEnabled(true);
    EXPECT_TRUE(filter->isEnabled());
}