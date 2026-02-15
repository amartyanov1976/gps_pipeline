#pragma once

#include <deque>
#include <optional>
#include <mutex>
#include "gps_point.h"

class GpsHistory {
public:
    explicit GpsHistory(size_t maxSize = 10);
    ~GpsHistory();
    
    // Добавить точку в историю
    void addPoint(const GpsPoint& point);
    
    // Получить последнюю валидную точку
    std::optional<GpsPoint> getLastValid() const;
    
    // Получить все точки истории
    std::deque<GpsPoint> getAllPoints() const;
    
    // Очистить историю
    void clear();
    
    // Получить размер истории
    size_t size() const;
    
    // Проверить, пуста ли история
    bool empty() const;
    
    // Установить максимальный размер истории
    void setMaxSize(size_t maxSize);
    
    // Получить максимальный размер истории
    size_t getMaxSize() const;
    
private:
    size_t maxSize_;
    std::deque<GpsPoint> points_;
    mutable std::mutex mutex_;
};