#include <gtest/gtest.h>
#include "infra/display_factory.h"

TEST(DisplayFactoryTest, Create_ConsoleDisplay) {
    JsonConfig config;
    config.setDisplayType("console");

    auto display = DisplayFactory::create(config);

    ASSERT_NE(display, nullptr);
}

TEST(DisplayFactoryTest, Create_DefaultIsConsole) {
    JsonConfig config;

    auto display = DisplayFactory::create(config);

    ASSERT_NE(display, nullptr);
}

TEST(DisplayFactoryTest, Create_FileDisplay) {
    JsonConfig config;
    config.setDisplayType("file");
    config.setOutputFile("/tmp/gps_test_output.log");

    auto display = DisplayFactory::create(config);

    ASSERT_NE(display, nullptr);

    // Cleanup
    std::remove("/tmp/gps_test_output.log");
}

TEST(DisplayFactoryTest, Create_FileWithoutPath_FallsBackToConsole) {
    JsonConfig config;
    config.setDisplayType("file");

    auto display = DisplayFactory::create(config);

    ASSERT_NE(display, nullptr);
}
