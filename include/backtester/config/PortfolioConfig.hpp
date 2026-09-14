#pragma once

#include <cstdint>

struct PortfolioConfig {
    uint32_t account_id;
    double account_balance;

    PortfolioConfig(uint32_t account_id, double account_balance)
        : account_id{account_id}, account_balance{account_balance} {}
};
