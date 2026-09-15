#pragma once

struct Metrics {
    Metrics(double account_balance = 0, double realized_pnl = 0, double unrealized_pnl = 0,
            double win_rate = 0, double maximum_drawdown = 0, double profit_factor = 0,
            double expectancy_money = 0, double expectancy_r = 0, int amount_of_orders = 0,
            int amount_of_signals = 0, int amount_of_executed_orders = 0)
        : account_balance{account_balance}, realized_pnl{realized_pnl},
          unrealized_pnl{unrealized_pnl}, win_rate{win_rate}, maximum_drawdown{maximum_drawdown},
          profit_factor{profit_factor}, expectancy_money{expectancy_money},
          expectancy_r{expectancy_r}, amount_of_orders{amount_of_orders},
          amount_of_signals{amount_of_signals},
          amount_of_executed_orders{amount_of_executed_orders} {}

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
