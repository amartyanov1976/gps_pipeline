#include "infra/filter_factory.h"
#include "infra/satellite_filter.h"
#include "infra/speed_filter.h"
#include "infra/jump_filter.h"
#include "infra/stop_filter.h"
#include "infra/smoothing_filter.h"
#include <iostream>

std::unique_ptr<IGpsFilter> FilterFactory::create(const FilterConfig& config) {
    std::unique_ptr<IGpsFilter> filter;

    if (config.type == "SatelliteFilter") {
        int minSatellites = 4;
        auto it = config.params.find("minSatellites");
        if (it != config.params.end()) {
            minSatellites = static_cast<int>(it->second);
        }
        filter = std::make_unique<SatelliteFilter>(minSatellites);
    }
    else if (config.type == "SpeedFilter") {
        double maxSpeed = 300.0;
        auto it = config.params.find("maxSpeed");
        if (it != config.params.end()) {
            maxSpeed = it->second;
        }
        filter = std::make_unique<SpeedFilter>(maxSpeed);
    }
    else if (config.type == "JumpFilter") {
        double maxJump = 100.0;
        auto it = config.params.find("maxJump");
        if (it != config.params.end()) {
            maxJump = it->second;
        }
        filter = std::make_unique<JumpFilter>(maxJump);
    }
    else if (config.type == "StopFilter") {
        double threshold = 3.0;
        auto it = config.params.find("threshold");
        if (it != config.params.end()) {
            threshold = it->second;
        }
        filter = std::make_unique<StopFilter>(threshold);
    }
    else if (config.type == "SmoothingFilter") {
        double cutoff = 0.1;
        double sampleRate = 1.0;

        auto it = config.params.find("cutoffFrequency");
        if (it != config.params.end()) {
            cutoff = it->second;
        }

        it = config.params.find("sampleRate");
        if (it != config.params.end()) {
            sampleRate = it->second;
        }

        filter = std::make_unique<SmoothingFilter>(cutoff, sampleRate);
    }
    else {
        std::cerr << "Warning: Unknown filter type '" << config.type << "'\n";
        return nullptr;
    }

    filter->setEnabled(config.enabled);
    return filter;
}
