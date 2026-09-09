#pragma once

#include "backtester/events/FillEvent.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/events/OrderEvent.hpp"
#include "backtester/events/SignalEvent.hpp"
#include "backtester/models/Metrics.hpp"
#include "backtester/models/Position.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include <cstdint>
#include <functional>
#include <memory>

class MetricCalculator {
  public:
    MetricCalculator(const Portfolio &portfolio);

    void on_market_event(const std::shared_ptr<MarketEvent> &event);

    void on_fill_event(const std::shared_ptr<FillEvent> &event);

    void on_signal_event(const std::shared_ptr<SignalEvent> &event);

    void on_order_event(const std::shared_ptr<OrderEvent> &event);

    Metrics calculate_metrics() const;

  private:
    void update_total_amount_of_trades();

    void update_maximum_drawdown();

    void update_win_and_loss(const Position &position);

    void update_r_multipliers_sum(const Position &position);

    static double calculate_r_multiplier(const Position &position);

    std::reference_wrapper<const Portfolio> portfolio_;

    double max_drawdown_;

    uint32_t total_amount_of_trades_ = 0;

    uint32_t amount_of_winning_trades_ = 0;

    uint32_t amount_of_loss_trades_ = 0;

    double gross_profit_ = 0;

    double gross_loss_ = 0;

    double r_multipliers_sum_ = 0;

    int amount_of_orders_ = 0;

    int amount_of_signals_ = 0;

    int amount_of_executed_orders_ = 0;
};
