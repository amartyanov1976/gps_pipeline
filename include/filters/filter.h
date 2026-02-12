#pragma once

#include "gps_point.h"
#include <deque>
#include <string>
#include <memory>

namespace gps {

enum class FilterResult {
    PASS,   // Точка валидна, передать дальше
    REJECT, // Точка невалидна, отбросить
    STOP    // Точка обработана, прекратить цепочку
};

class Filter {
public:
    virtual ~Filter() = default;
    
    // Основной метод обработки
    virtual FilterResult process(GpsPoint& point, const std::deque<GpsPoint>& history) = 0;
    
    // Получение имени фильтра
    virtual std::string name() const = 0;
    
    // Управление состоянием
    virtual void setEnabled(bool enabled);
    virtual bool isEnabled() const;
    
    // Сброс состояния фильтра
    virtual void reset();
    
protected:
    bool enabled_ = true;
};

} // namespace gps