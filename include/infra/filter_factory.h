#pragma once

#include <memory>
#include "domain/i_filter.h"
#include "infra/json_config.h"

class FilterFactory {
public:
    static std::unique_ptr<IGpsFilter> create(const FilterConfig& config);
};
