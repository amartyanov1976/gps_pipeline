#include "filters/filter.h"

namespace gps {

void Filter::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool Filter::isEnabled() const {
    return enabled_;
}

void Filter::reset() {
    // Базовая реализация - ничего не делает
    // Переопределяется в наследниках при необходимости
}

} // namespace gps