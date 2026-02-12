cmake_minimum_required(VERSION 3.15)
project(gps_pipeline VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Пути
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# Поиск libpipeline
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPIPELINE REQUIRED libpipeline)

# Настройки компилятора
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror=return-type)
endif()

# Санитайзеры
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# Покрытие кода
if(ENABLE_COVERAGE)
    include(cmake/code-coverage.cmake)
endif()

# Основная библиотека
add_library(gps_pipeline_lib
    src/gps_point.cpp
    src/nmea_parser.cpp
    src/display.cpp
    src/pipeline.cpp
    src/filters/filter.cpp
    src/filters/satellite_filter.cpp
    src/filters/speed_filter.cpp
    src/filters/jump_filter.cpp
    src/filters/stop_filter.cpp
)

target_include_directories(gps_pipeline_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${LIBPIPELINE_INCLUDE_DIRS}
)

target_compile_options(gps_pipeline_lib
    PRIVATE ${LIBPIPELINE_CFLAGS_OTHER}
)

target_link_libraries(gps_pipeline_lib
    PUBLIC
        ${LIBPIPELINE_LIBRARIES}
)

# Исполняемый файл
if(BUILD_EXAMPLES)
    add_executable(gps_pipeline
        src/main.cpp
    )
    
    target_link_libraries(gps_pipeline
        PRIVATE
            gps_pipeline_lib
    )
endif()

# Тесты
if(BUILD_TESTS)
    enable_testing()
    
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
    
    add_executable(gps_pipeline_tests
        tests/test_main.cpp
        tests/test_gps_point.cpp
        tests/test_nmea_parser.cpp
        tests/test_display.cpp
        tests/test_pipeline.cpp
        tests/filters/test_filter.cpp
        tests/filters/test_satellite_filter.cpp
        tests/filters/test_speed_filter.cpp
        tests/filters/test_jump_filter.cpp
        tests/filters/test_stop_filter.cpp
    )
    
    target_include_directories(gps_pipeline_tests
        PRIVATE
            include
            ${LIBPIPELINE_INCLUDE_DIRS}
    )
    
    target_link_libraries(gps_pipeline_tests
        PRIVATE
            gps_pipeline_lib
            GTest::gtest_main
            GTest::gmock
    )
    
    if(ENABLE_COVERAGE)
        setup_coverage_targets(gps_pipeline_tests)
    endif()
    
    add_test(NAME gps_pipeline_tests COMMAND gps_pipeline_tests)
endif()

# Установка
install(TARGETS gps_pipeline_lib gps_pipeline
    EXPORT gps_pipelineConfig
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/gps_pipeline
)

install(EXPORT gps_pipelineConfig
    DESTINATION lib/cmake/gps_pipeline
)

# CPack
set(CPACK_GENERATOR "TGZ;DEB;RPM")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)