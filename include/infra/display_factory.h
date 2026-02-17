#pragma once

#include <memory>
#include "domain/i_display.h"
#include "infra/json_config.h"

class DisplayFactory {
public:
    static std::unique_ptr<IDisplay> create(const JsonConfig& config);
};
