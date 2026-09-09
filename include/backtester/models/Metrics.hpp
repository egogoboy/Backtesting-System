#pragma once

struct Metrics {
    double account_balance = 0;
    double realized_pnl = 0;
    double unrealized_pnl = 0;
    double win_rate = 0;
    double maximum_drawdown = 0;
    double profit_factor = 0;
    double expectancy_money = 0;
    double expectancy_r = 0;
    int amount_of_orders = 0;
    int amount_of_signals = 0;
    int amount_of_executed_orders = 0;
};
