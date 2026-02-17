cmake_minimum_required(VERSION 3.10)
project(GpsPipeline VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# === Domain layer (entities + interfaces, header-only) ===
add_library(gps_domain
    src/gps_point.cpp
)
target_include_directories(gps_domain PUBLIC include)

# === Infrastructure layer (implementations) ===
add_library(gps_infra
    src/nmea_parser.cpp
    src/gps_history.cpp
    src/satellite_filter.cpp
    src/speed_filter.cpp
    src/jump_filter.cpp
    src/stop_filter.cpp
    src/smoothing_filter.cpp
    src/console_display.cpp
    src/file_display.cpp
    src/json_config.cpp
    src/filter_factory.cpp
    src/display_factory.cpp
)
target_link_libraries(gps_infra PUBLIC gps_domain)

# === Application layer (use cases) ===
add_library(gps_app
    src/pipeline.cpp
)
target_link_libraries(gps_app PUBLIC gps_domain)

# === Executable (composition root) ===
add_executable(gps_pipeline main.cpp)
target_link_libraries(gps_pipeline gps_app gps_infra)

# === Tests ===
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
        tests/test_smoothing_filter.cpp
        tests/test_console_display.cpp
        tests/test_mock_display.cpp
        tests/test_pipeline.cpp
        tests/test_filter_factory.cpp
        tests/test_display_factory.cpp
    )

    target_include_directories(gps_tests PRIVATE
        include
        tests
    )

    target_link_libraries(gps_tests
        gps_app
        gps_infra
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
    )

    add_test(NAME GpsTests COMMAND gps_tests)
endif()
