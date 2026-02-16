# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(GpsPipeline VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(BUILD_TESTS "Build tests" ON)

# Установка путей
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Основная библиотека
add_library(gps_core 
    src/gps_point.cpp
    src/parser.cpp
    src/history.cpp
    src/satellite_filter.cpp
    src/speed_filter.cpp
    src/jump_filter.cpp
    src/stop_filter.cpp
    src/console_display.cpp
    src/mock_display.cpp
    src/pipeline.cpp
    src/smoothing_filter.cpp
    src/json_config.cpp
    src/file_display.cpp
)

target_include_directories(gps_core PUBLIC include)

# Исполняемый файл
add_executable(gps_pipeline main.cpp)
target_link_libraries(gps_pipeline gps_core)

# Тесты
if(BUILD_TESTS)
    enable_testing()
    
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)
    
    add_executable(gps_tests
        tests/test_parser.cpp
        tests/test_history.cpp
        tests/test_satellite_filter.cpp
        tests/test_speed_filter.cpp
        tests/test_jump_filter.cpp
        tests/test_stop_filter.cpp
        tests/test_console_display.cpp
        tests/test_mock_display.cpp
        tests/test_pipeline.cpp
        tests/test_smoothing_filter.cpp
    )
    
    target_include_directories(gps_tests PRIVATE include)
    target_link_libraries(gps_tests 
        gps_core 
        GTest::gtest 
        GTest::gtest_main 
        Threads::Threads
    )
    
    add_test(NAME GpsTests COMMAND gps_tests)
endif()

