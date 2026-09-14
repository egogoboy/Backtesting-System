#pragma once

struct ExecutionConfig {
    double margin_rate;
    double maintenance_margin_rate;

    ExecutionConfig(double margin_rate, double maintenance_margin_rate)
        : margin_rate{margin_rate}, maintenance_margin_rate{maintenance_margin_rate} {}
};
