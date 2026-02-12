#pragma once

#include "gps_point.h"
#include <iostream>
#include <string>
#include <memory>

namespace gps {

class Display {
public:
    virtual ~Display() = default;
    virtual void show(const GpsPoint& point) = 0;
    virtual void error(const std::string& message) = 0;
    virtual void message(const std::string& message) = 0;
};

class ConsoleDisplay : public Display {
public:
    explicit ConsoleDisplay(std::ostream& out = std::cout);
    void show(const GpsPoint& point) override;
    void error(const std::string& message) override;
    void message(const std::string& message) override;

private:
    std::ostream& out_;
};

class MockDisplay : public Display {
public:
    void show(const GpsPoint& point) override;
    void error(const std::string& message) override;
    void message(const std::string& message) override;
    
    const GpsPoint& getLastPoint() const;
    const std::string& getLastError() const;
    const std::string& getLastMessage() const;
    void clear();

private:
    GpsPoint lastPoint_;
    std::string lastError_;
    std::string lastMessage_;
};

} // namespace gps