#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <fstream>
#include "output/console_output.h"
#include "output/mock_output.h"

using namespace testing;

// ============================================================================
// Тесты консольного вывода
// ============================================================================

class ConsoleOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Сохраняем старый буфер
        old_cout = std::cout.rdbuf();
        std::cout.rdbuf(stream.rdbuf());
    }
    
    void TearDown() override {
        // Восстанавливаем буфер
        std::cout.rdbuf(old_cout);
    }
    
    std::stringstream stream;
    std::streambuf* old_cout;
    
    GpsPoint createTestPoint() {
        GpsPoint p;
        p.latitude = 55.7558;
        p.longitude = 37.6176;
        p.speed = 25.5;
        p.timestamp = 123456789;
        p.satellites = 8;
        p.quality = 1;
        p.type = "GGA";
        return p;
    }
};

TEST_F(ConsoleOutputTest, WritePoint) {
    ConsoleOutput output;
    GpsPoint point = createTestPoint();
    
    output.write(point);
    
    std::string result = stream.str();
    
    // Проверяем наличие всех полей
    EXPECT_THAT(result, HasSubstr("55.7558"));
    EXPECT_THAT(result, HasSubstr("37.6176"));
    EXPECT_THAT(result, HasSubstr("25.5"));
    EXPECT_THAT(result, HasSubstr("8"));
    EXPECT_THAT(result, HasSubstr("GGA"));
    EXPECT_THAT(result, HasSubstr("123456789"));
}

TEST_F(ConsoleOutputTest, WriteMultiplePoints) {
    ConsoleOutput output;
    
    for (int i = 0; i < 5; i++) {
        GpsPoint point = createTestPoint();
        point.timestamp = 1000 + i;
        point.speed = i * 10;
        output.write(point);
    }
    
    std::string result = stream.str();
    int line_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(line_count, 5); // 5 строк
}

TEST_F(ConsoleOutputTest, WritePointWithMinimalData) {
    ConsoleOutput output;
    GpsPoint point;
    point.latitude = 55.7558;
    point.longitude = 37.6176;
    // Остальные поля не заполнены
    
    output.write(point);
    
    std::string result = stream.str();
    EXPECT_THAT(result, HasSubstr("55.7558"));
    EXPECT_THAT(result, HasSubstr("37.6176"));
    EXPECT_FALSE(result.empty());
}

TEST_F(ConsoleOutputTest, WritePointWithNegativeCoordinates) {
    ConsoleOutput output;
    GpsPoint point = createTestPoint();
    point.latitude = -33.8688;  // Сидней
    point.longitude = 151.2093;
    
    output.write(point);
    
    std::string result = stream.str();
    EXPECT_THAT(result, HasSubstr("-33.8688"));
    EXPECT_THAT(result, HasSubstr("151.2093"));
}

TEST_F(ConsoleOutputTest, WritePointWithZeroValues) {
    ConsoleOutput output;
    GpsPoint point;
    point.latitude = 0.0;
    point.longitude = 0.0;
    point.speed = 0.0;
    point.timestamp = 0;
    point.satellites = 0;
    
    output.write(point);
    
    std::string result = stream.str();
    EXPECT_THAT(result, HasSubstr("0"));
}

TEST_F(ConsoleOutputTest, DifferentOutputFormats) {
    ConsoleOutput output;
    GpsPoint point = createTestPoint();
    
    // Если есть поддержка разных форматов, проверяем их
    // В данном случае просто проверяем, что метод не падает
    
    for (int i = 0; i < 3; i++) {
        output.write(point);
    }
    
    std::string result = stream.str();
    EXPECT_FALSE(result.empty());
}

TEST_F(ConsoleOutputTest, LargeNumberOfPoints) {
    ConsoleOutput output;
    const int NUM_POINTS = 1000;
    
    for (int i = 0; i < NUM_POINTS; i++) {
        GpsPoint point = createTestPoint();
        point.timestamp = i;
        point.speed = i * 0.1;
        output.write(point);
    }
    
    std::string result = stream.str();
    int line_count = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(line_count, NUM_POINTS);
}

// ============================================================================
// Тесты mock вывода
// ============================================================================

class MockOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock = std::make_shared<MockOutput>();
    }
    
    std::shared_ptr<MockOutput> mock;
    
    GpsPoint createTestPoint() {
        GpsPoint p;
        p.latitude = 55.7558;
        p.longitude = 37.6176;
        p.speed = 25.5;
        p.timestamp = 123456;
        p.satellites = 8;
        p.quality = 1;
        p.type = "GGA";
        return p;
    }
};

TEST_F(MockOutputTest, Constructor) {
    EXPECT_EQ(mock->getWriteCount(), 0);
    EXPECT_FALSE(mock->getLastWritten().has_value());
}

TEST_F(MockOutputTest, WriteSinglePoint) {
    GpsPoint point = createTestPoint();
    mock->write(point);
    
    EXPECT_EQ(mock->getWriteCount(), 1);
    
    auto last = mock->getLastWritten();
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(last->timestamp, point.timestamp);
    EXPECT_DOUBLE_EQ(last->latitude, point.latitude);
    EXPECT_DOUBLE_EQ(last->longitude, point.longitude);
    EXPECT_DOUBLE_EQ(last->speed, point.speed);
    EXPECT_EQ(last->satellites, point.satellites);
}

TEST_F(MockOutputTest, WriteMultiplePoints) {
    const int NUM_POINTS = 10;
    
    for (int i = 0; i < NUM_POINTS; i++) {
        GpsPoint point = createTestPoint();
        point.timestamp = 1000 + i;
        point.speed = i * 10;
        mock->write(point);
    }
    
    EXPECT_EQ(mock->getWriteCount(), NUM_POINTS);
    
    auto last = mock->getLastWritten();
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(last->timestamp, 1000 + NUM_POINTS - 1);
    EXPECT_DOUBLE_EQ(last->speed, (NUM_POINTS - 1) * 10);
}

TEST_F(MockOutputTest, Clear) {
    for (int i = 0; i < 5; i++) {
        mock->write(createTestPoint());
    }
    
    EXPECT_EQ(mock->getWriteCount(), 5);
    EXPECT_TRUE(mock->getLastWritten().has_value());
    
    mock->clear();
    
    EXPECT_EQ(mock->getWriteCount(), 0);
    EXPECT_FALSE(mock->getLastWritten().has_value());
}

TEST_F(MockOutputTest, GetAllPoints) {
    std::vector<GpsPoint> points;
    
    for (int i = 0; i < 5; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = 1000 + i;
        p.speed = i * 10;
        points.push_back(p);
        mock->write(p);
    }
    
    auto all = mock->getAllPoints();
    ASSERT_EQ(all.size(), 5);
    
    for (size_t i = 0; i < all.size(); i++) {
        EXPECT_EQ(all[i].timestamp, points[i].timestamp);
        EXPECT_DOUBLE_EQ(all[i].speed, points[i].speed);
    }
}

TEST_F(MockOutputTest, FindPoint) {
    for (int i = 0; i < 10; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = 1000 + i;
        p.speed = i * 10;
        mock->write(p);
    }
    
    // Поиск по таймстемпу
    auto found = mock->findPoint(1005);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->timestamp, 1005);
    EXPECT_DOUBLE_EQ(found->speed, 50);
    
    // Поиск несуществующего
    found = mock->findPoint(2000);
    EXPECT_FALSE(found.has_value());
}

TEST_F(MockOutputTest, FindPointByPredicate) {
    for (int i = 0; i < 10; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = 1000 + i;
        p.speed = i * 10;
        p.satellites = (i % 2 == 0) ? 8 : 4;
        mock->write(p);
    }
    
    // Поиск по предикату
    auto found = mock->findPoint([](const GpsPoint& p) {
        return p.satellites == 8 && p.speed > 30;
    });
    
    ASSERT_TRUE(found.has_value());
    EXPECT_GT(found->speed, 30);
    EXPECT_EQ(found->satellites, 8);
    
    // Поиск несуществующего
    found = mock->findPoint([](const GpsPoint& p) {
        return p.speed > 100;
    });
    EXPECT_FALSE(found.has_value());
}

TEST_F(MockOutputTest, FilterPoints) {
    for (int i = 0; i < 20; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = 1000 + i;
        p.speed = i * 5;
        p.satellites = (i % 2 == 0) ? 8 : 4;
        p.quality = (i % 3 == 0) ? 1 : 2;
        mock->write(p);
    }
    
    // Фильтр по минимальной скорости
    auto filtered = mock->filterPoints([](const GpsPoint& p) {
        return p.speed > 50.0;
    });
    
    EXPECT_EQ(filtered.size(), 10); // speed > 50: 55,60,65,70,75,80,85,90,95
    
    for (const auto& p : filtered) {
        EXPECT_GT(p.speed, 50.0);
    }
    
    // Фильтр по спутникам
    filtered = mock->filterPoints([](const GpsPoint& p) {
        return p.satellites == 8;
    });
    
    EXPECT_EQ(filtered.size(), 10); // четные индексы
    
    for (const auto& p : filtered) {
        EXPECT_EQ(p.satellites, 8);
    }
    
    // Комбинированный фильтр
    filtered = mock->filterPoints([](const GpsPoint& p) {
        return p.satellites == 8 && p.quality == 1;
    });
    
    // Должны быть точки с четным индексом и кратным 3
    // 0,6,12,18
    EXPECT_EQ(filtered.size(), 4);
}

TEST_F(MockOutputTest, GetPointsInTimeRange) {
    for (int i = 0; i < 20; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = 1000 + i * 10; // 1000,1010,1020,...
        mock->write(p);
    }
    
    auto range = mock->getPointsInTimeRange(1020, 1050);
    EXPECT_EQ(range.size(), 4); // 1020,1030,1040,1050
    
    for (const auto& p : range) {
        EXPECT_GE(p.timestamp, 1020);
        EXPECT_LE(p.timestamp, 1050);
    }
    
    // Пустой диапазон
    range = mock->getPointsInTimeRange(2000, 2010);
    EXPECT_TRUE(range.empty());
}

TEST_F(MockOutputTest, GetPointsByQuality) {
    for (int i = 0; i < 10; i++) {
        GpsPoint p = createTestPoint();
        p.quality = i % 3; // 0,1,2,0,1,2,...
        mock->write(p);
    }
    
    auto quality1 = mock->getPointsByQuality(1);
    EXPECT_EQ(quality1.size(), 3); // позиции 1,4,7
    
    auto quality2 = mock->getPointsByQuality(2);
    EXPECT_EQ(quality2.size(), 3); // позиции 2,5,8
    
    auto quality0 = mock->getPointsByQuality(0);
    EXPECT_EQ(quality0.size(), 4); // позиции 0,3,6,9
}

TEST_F(MockOutputTest, GetPointsWithMinSatellites) {
    for (int i = 0; i < 10; i++) {
        GpsPoint p = createTestPoint();
        p.satellites = i; // 0..9
        mock->write(p);
    }
    
    auto points = mock->getPointsWithMinSatellites(5);
    EXPECT_EQ(points.size(), 5); // 5,6,7,8,9
    
    points = mock->getPointsWithMinSatellites(10);
    EXPECT_TRUE(points.empty());
}

TEST_F(MockOutputTest, Statistics) {
    for (int i = 0; i < 10; i++) {
        GpsPoint p = createTestPoint();
        p.speed = i * 10; // 0,10,20,...,90
        p.satellites = i + 1; // 1..10
        p.quality = (i % 2) + 1; // 1,2,1,2,...
        mock->write(p);
    }
    
    auto stats = mock->getStatistics();
    
    EXPECT_EQ(stats.total_points, 10);
    EXPECT_DOUBLE_EQ(stats.avg_speed, 45.0); // (0+10+...+90)/10
    EXPECT_DOUBLE_EQ(stats.max_speed, 90.0);
    EXPECT_DOUBLE_EQ(stats.min_speed, 0.0);
    EXPECT_DOUBLE_EQ(stats.avg_satellites, 5.5); // (1+2+...+10)/10
    EXPECT_EQ(stats.min_satellites, 1);
    EXPECT_EQ(stats.max_satellites, 10);
    
    // Проверка распределения по качеству
    EXPECT_EQ(stats.quality_distribution[1], 5);
    EXPECT_EQ(stats.quality_distribution[2], 5);
}

TEST_F(MockOutputTest, ExportToJson) {
    for (int i = 0; i < 3; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = 1000 + i;
        p.speed = i * 10;
        mock->write(p);
    }
    
    std::string json = mock->exportToJson();
    
    // Проверяем, что JSON содержит ожидаемые поля
    EXPECT_THAT(json, HasSubstr("timestamp"));
    EXPECT_THAT(json, HasSubstr("latitude"));
    EXPECT_THAT(json, HasSubstr("longitude"));
    EXPECT_THAT(json, HasSubstr("speed"));
    EXPECT_THAT(json, HasSubstr("1000"));
    EXPECT_THAT(json, HasSubstr("1001"));
    EXPECT_THAT(json, HasSubstr("1002"));
}

TEST_F(MockOutputTest, ExportToCsv) {
    for (int i = 0; i < 3; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = 1000 + i;
        p.speed = i * 10;
        mock->write(p);
    }
    
    std::string csv = mock->exportToCsv();
    
    // Проверяем CSV структуру
    int line_count = std::count(csv.begin(), csv.end(), '\n');
    EXPECT_EQ(line_count, 4); // заголовок + 3 строки
    
    EXPECT_THAT(csv, HasSubstr("timestamp,latitude,longitude,speed"));
    EXPECT_THAT(csv, HasSubstr("1000"));
    EXPECT_THAT(csv, HasSubstr("1001"));
    EXPECT_THAT(csv, HasSubstr("1002"));
}

TEST_F(MockOutputTest, Reset) {
    for (int i = 0; i < 5; i++) {
        mock->write(createTestPoint());
    }
    
    EXPECT_EQ(mock->getWriteCount(), 5);
    
    mock->reset();
    
    EXPECT_EQ(mock->getWriteCount(), 0);
    EXPECT_FALSE(mock->getLastWritten().has_value());
    EXPECT_TRUE(mock->getAllPoints().empty());
}

TEST_F(MockOutputTest, WriteAfterClear) {
    mock->write(createTestPoint());
    EXPECT_EQ(mock->getWriteCount(), 1);
    
    mock->clear();
    EXPECT_EQ(mock->getWriteCount(), 0);
    
    mock->write(createTestPoint());
    EXPECT_EQ(mock->getWriteCount(), 1);
}

TEST_F(MockOutputTest, LargeNumberOfWrites) {
    const int NUM_POINTS = 10000;
    
    for (int i = 0; i < NUM_POINTS; i++) {
        GpsPoint p = createTestPoint();
        p.timestamp = i;
        mock->write(p);
    }
    
    EXPECT_EQ(mock->getWriteCount(), NUM_POINTS);
    
    auto last = mock->getLastWritten();
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(last->timestamp, NUM_POINTS - 1);
}

TEST_F(MockOutputTest, ConcurrentWrites) {
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    for (int t = 0; t < 4; t++) {
        threads.emplace_back([this, t, &success_count]() {
            for (int i = 0; i < 100; i++) {
                GpsPoint p = createTestPoint();
                p.timestamp = t * 1000 + i;
                mock->write(p);
                success_count++;
            }
        });
    }
    
    for (auto& th : threads) {
        th.join();
    }
    
    EXPECT_EQ(mock->getWriteCount(), 400);
    EXPECT_TRUE(mock->getLastWritten().has_value());
}

// ============================================================================
// Тесты для абстрактного класса Output (если есть)
// ============================================================================

// Эти тесты проверяют, что все классы вывода правильно реализуют интерфейс

class OutputInterfaceTest : public testing::TestWithParam<std::shared_ptr<Output>> {
protected:
    void SetUp() override {
        output = GetParam();
    }
    
    std::shared_ptr<Output> output;
    
    GpsPoint createTestPoint() {
        GpsPoint p;
        p.latitude = 55.7558;
        p.longitude = 37.6176;
        p.timestamp = 123456;
        return p;
    }
};

TEST_P(OutputInterfaceTest, WriteDoesNotThrow) {
    GpsPoint point = createTestPoint();
    EXPECT_NO_THROW(output->write(point));
}

TEST_P(OutputInterfaceTest, MultipleWritesDoNotThrow) {
    for (int i = 0; i < 10; i++) {
        GpsPoint point = createTestPoint();
        point.timestamp = i;
        EXPECT_NO_THROW(output->write(point));
    }
}

// Инстанцирование тестов для разных реализаций
INSTANTIATE_TEST_SUITE_P(
    OutputImplementations,
    OutputInterfaceTest,
    Values(
        std::make_shared<ConsoleOutput>(),
        std::make_shared<MockOutput>()
    )
);