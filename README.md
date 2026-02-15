# GPS Data Processing Pipeline

## 1. Описание

Проект реализует полный пайплайн обработки GPS-данных в формате NMEA 0183. 
Система выполняет парсинг NMEA-строк (RMC и GGA сообщения) с валидацией 
контрольной суммы, конвертирует координаты из формата DDMM.MMM в десятичные 
градусы и скорость из узлов в км/ч. Полученные точки проходят через цепочку 
фильтров с настраиваемыми приоритетами, результаты выводятся в консоль. 
Проект разработан с соблюдением принципов SOLID и методологии TDD.

## 2. Сборка

### Требования
- CMake 3.10 или выше
- Компилятор с поддержкой C++17
- Google Test (для сборки тестов)

### Команды для сборки

# Клонирование репозитория
git clone https://github.com/amartyanov1976/gps_pipeline
cd gps-pipeline

# Создание директории для сборки
mkdir build && cd build

# Конфигурация CMake
cmake ..

# Сборка проекта
cmake --build .

## 3. Запуск
Запуск демо

# Из директории build
./bin/gps_pipeline ../data/sample.nmea
Запуск тестов

# Из директории build
ctest --output-on-failure

# Или напрямую
./bin/gps_tests

## 4. Архитектура
text
┌─────────────┐    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   Источник  │───▶│   Парсер    │───▶│   Фильтры   │───▶│   Вывод     │
│   (NMEA)    │    │   (NMEA)    │    │  (Pipeline) │    │  (Display)  │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
Основные компоненты:

GpsPoint - структура данных точки

NmeaParser - парсинг RMC и GGA сообщений

GpsHistory - история последних N валидных точек

Фильтры (SatelliteFilter, SpeedFilter, JumpFilter, StopFilter)

IDisplay - интерфейс вывода (ConsoleDisplay, MockDisplay)

GpsPipeline - основной класс, объединяющий все компоненты

## 5. Конфигурация фильтров
В main.cpp можно настроить параметры фильтров:

cpp
// Создание пайплайна
auto display = std::make_unique<ConsoleDisplay>();
GpsPipeline pipeline(std::move(display));

// Настройка фильтров
auto satelliteFilter = std::make_unique<SatelliteFilter>(4);  // минимум 4 спутника
auto speedFilter = std::make_unique<SpeedFilter>(300.0);      // макс 300 км/ч
auto jumpFilter = std::make_unique<JumpFilter>(100.0);        // макс скачок 100 м
auto stopFilter = std::make_unique<StopFilter>(3.0);          // порог остановки 3 км/ч

// Добавление фильтров с приоритетами (меньше = раньше)
pipeline.addFilter(std::move(satelliteFilter), 1);
pipeline.addFilter(std::move(speedFilter), 2);
pipeline.addFilter(std::move(jumpFilter), 3);
pipeline.addFilter(std::move(stopFilter), 4);

// Настройка истории
pipeline.setHistorySize(10);  // хранить последние 10 точек
Параметры фильтров
Фильтр			Параметр			Описание				По умолчанию
SatelliteFilter	minSatellites		Минимум спутников		4
SpeedFilter		maxSpeedKmh			Макс. скорость (км/ч)	300.0
JumpFilter		maxJumpMeters		Макс. скачок (метры)	100.0
StopFilter		speedThresholdKmh	Порог остановки (км/ч)	3.0
