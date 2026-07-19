#pragma once

#include "backtester/core/EventQueue.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/events/OrderEvent.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Order.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include <functional>
#include <random>
#include <vector>

class ExecutionHandler {
  public:
    ExecutionHandler(EventQueue &event_queue, Portfolio &portfolio,
                     const MarketData &initial_market_data);

    void on_market_event(const std::shared_ptr<MarketEvent> &event);

    void on_order_event(const std::shared_ptr<OrderEvent> &event);

    double get_floating_risk() const;

  private:
    void execute_order(Order &order);

    static bool can_execute_order(const Order &order, const MarketData &market_data);

    void fill_position(Order &order);

    void update_floating_risk(const Order &order);

    double calculate_spread();

    void update_atr(const MarketData &market_data);

    double calculate_true_range(const MarketData &market_data);

    double floating_risk_ = 0;

    double atr_ = 0;
    uint32_t number_of_periods_ = 0;
    const double ATR_PERIOD = 14;

    std::mt19937_64 rng_;
    static constexpr size_t SPREAD_SEED = 42;

    static constexpr double MINIMAL_SPREAD = 1e-4;
    static constexpr double MAXIMUM_SPREAD = 1e-4 * 5;
    std::uniform_real_distribution<double> spread_dist_{MINIMAL_SPREAD, MAXIMUM_SPREAD};

    std::vector<Order> orders_;
    std::vector<Position> positions_;

    std::reference_wrapper<EventQueue> event_queue_;
    std::reference_wrapper<Portfolio> portfolio_;
    std::reference_wrapper<const MarketData> last_market_data_;
};
