# GPS Data Processing Pipeline

## 1. Описание

Проект реализует полный пайплайн обработки GPS-данных в формате NMEA 0183. 
Система выполняет парсинг NMEA-строк (RMC, GGA и GSV сообщения) с валидацией 
контрольной суммы, конвертирует координаты из формата DDMM.MMM в десятичные 
градусы и скорость из узлов в км/ч. Полученные точки проходят через цепочку 
фильтров с настраиваемыми приоритетами, результаты выводятся в консоль. 
Программа автоматически загружает конфигурацию из файла ../data/config.json.

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
│   (NMEA)    │    │   (NMEA)    │    │             │    │  (Display)  │
└─────────────┘    └─────────────┘    └─────────────┘    └─────────────┘
         │                │                  │                   │
         ▼                ▼                  ▼                   ▼
   ┌─────────────────────────────────────────────────────────────────┐
   │                      Конфигурация (config.json)                 │
   └─────────────────────────────────────────────────────────────────┘
Основные компоненты:
GpsPoint - структура данных точки с метаданными (источник, статус фильтрации)

NmeaParser - парсинг RMC и GGA сообщений

GpsHistory - история последних N валидных точек

Фильтры (SatelliteFilter, SpeedFilter, JumpFilter, StopFilter, SmoothingFilter)

IDisplay - интерфейс вывода (ConsoleDisplay, FileDisplay, MockDisplay)

JsonConfig - загрузка и парсинг конфигурационного файла

GpsPipeline - основной класс, объединяющий все компоненты

## 5. Конфигурация

Настройка параметров производится через файл `data/config.json`:

```json
{
    "historySize": 20,
    "displayType": "file",
    "outputFile": "gps_output.log",
    "fileRotation": true,
    "maxFileSize": 1048576,
    "filters": [
        {
            "type": "SatelliteFilter",
            "enabled": true,
            "priority": 1,
            "params": {
                "minSatellites": 4
            }
        },
        {
            "type": "SpeedFilter",
            "enabled": true,
            "priority": 2,
            "params": {
                "maxSpeed": 250.0
            }
        },
        {
            "type": "JumpFilter",
            "enabled": true,
            "priority": 3,
            "params": {
                "maxJump": 50.0
            }
        },
        {
            "type": "StopFilter",
            "enabled": true,
            "priority": 4,
            "params": {
                "threshold": 2.5
            }
        },
        {
            "type": "SmoothingFilter",
            "enabled": true,
            "priority": 5,
            "params": {
                "cutoffFrequency": 0.2,
                "sampleRate": 2.0
            }
        }
    ]
}

Параметры конфигурации
Основные параметры
Параметр	Тип	Описание
historySize	integer	Количество последних точек, сохраняемых в истории (используется для фильтра скачков)
displayType	string	Тип вывода (console - вывод в консоль, file - запись в файл)
outputFile	string	Имя файла для записи результатов (используется при displayType: "file")
fileRotation	boolean	Включить/выключить ротацию файла при достижении максимального размера
maxFileSize	integer	Максимальный размер файла в байтах (для ротации)
Фильтры
Каждый фильтр в массиве filters содержит следующие поля:

Поле		Тип		Описание
type		string	Тип фильтра
enabled		boolean	Включить/выключить фильтр
priority	integer	Приоритет применения (меньше значение = раньше применяется)
params		object	Параметры фильтра (зависят от типа)

SatelliteFilter
Параметр		Тип		Описание
minSatellites	integer	Минимальное количество спутников для валидной точки

SpeedFilter
Параметр		Тип		Описание
maxSpeed		float	Максимальная допустимая скорость (км/ч)

JumpFilter
Параметр		Тип		Описание
maxJump			float	Максимальный допустимый скачок между последовательными точками (метры)

StopFilter
Параметр		Тип		Описание
threshold		float	Порог скорости для определения остановки (км/ч)

SmoothingFilter
Параметр		Тип		Описание
cutoffFrequency	float	Частота среза фильтра (Гц)
sampleRate		float	Частота дискретизации (Гц)

Приоритеты фильтров
Фильтры применяются в порядке возрастания приоритета (меньшее значение = раньше). В примере конфигурации:

SatelliteFilter (priority: 1) - первичная проверка количества спутников

SpeedFilter (priority: 2) - проверка скорости

JumpFilter (priority: 3) - анализ скачков координат

StopFilter (priority: 4) - обнаружение остановок

SmoothingFilter (priority: 5) - сглаживание траектории

Примеры использования
Консольный вывод с фильтрацией:

json
{
    "historySize": 10,
    "displayType": "console",
    "filters": [
        {
            "type": "SatelliteFilter",
            "enabled": true,
            "priority": 1,
            "params": { "minSatellites": 4 }
        }
    ]
}
Запись в файл с ротацией:

json
{
    "historySize": 20,
    "displayType": "file",
    "outputFile": "gps_track.log",
    "fileRotation": true,
    "maxFileSize": 5242880,
    "filters": []
}
Отключение всех фильтров:

json
{
    "historySize": 5,
    "displayType": "console",
    "filters": [
        {
            "type": "SatelliteFilter",
            "enabled": false,
            "priority": 1,
            "params": { "minSatellites": 4 }
        }
    ]
}