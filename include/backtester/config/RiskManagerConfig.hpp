#pragma once

struct RiskManagerConfig {
    double risk_per_trade;
    double total_account_risk;
    double sl_tp_ratio;

    RiskManagerConfig(double risk_per_trade, double total_account_risk, double sl_tp_ratio)
        : risk_per_trade{risk_per_trade}, total_account_risk{total_account_risk},
          sl_tp_ratio{sl_tp_ratio} {}
};
