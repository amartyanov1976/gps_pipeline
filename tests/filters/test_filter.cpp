#include <gtest/gtest.h>
#include "filters/filter.h"

using namespace gps;

class TestFilter : public Filter {
public:
    FilterResult process(GpsPoint&, const std::deque<GpsPoint>&) override {
        return FilterResult::PASS;
    }
    
    std::string name() const override {
        return "TestFilter";
    }
};

TEST(FilterTest, BaseFunctionality) {
    TestFilter filter;
    
    EXPECT_TRUE(filter.isEnabled());
    EXPECT_EQ(filter.name(), "TestFilter");
    
    filter.setEnabled(false);
    EXPECT_FALSE(filter.isEnabled());
    
    filter.setEnabled(true);
    EXPECT_TRUE(filter.isEnabled());
    
    filter.reset(); // Should not throw
}