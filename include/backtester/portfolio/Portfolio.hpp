#pragma once

#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/events/FillEvent.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/models/Position.hpp"

#include <cstdint>
#include <memory>
#include <vector>

class Portfolio {
  public:
    Portfolio(const PortfolioConfig &config)
        : account_id_{config.account_id}, start_account_balance_{config.account_balance},
          account_balance_{config.account_balance} {}

    void on_fill_event(const std::shared_ptr<FillEvent> &event);

    void on_market_event(const std::shared_ptr<MarketEvent> &event);

    bool reserve_margin(double amount);

    bool release_margin(double amount);

    bool charge_fee(double amount);

    bool has_available_funds(double amount) const;

    double get_floating_losses(double current_price) const;

    double get_realized_pnl() const;

    double get_unrealized_pnl() const;

    double get_account_balance() const;

    double get_total_equity() const;

  private:
    uint32_t account_id_;
    double start_account_balance_;
    double account_balance_;
    double realized_pnl_ = 0;
    double unrealized_pnl_ = 0;
    double reserved_funds_ = 0;

    std::vector<std::shared_ptr<const Position>> closed_positions_;
    std::vector<std::shared_ptr<const Position>> opened_positions_;
};
