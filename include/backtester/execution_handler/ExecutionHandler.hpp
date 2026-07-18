#pragma once

#include "backtester/core/EventQueue.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/events/OrderEvent.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Order.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include <functional>
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

    void fill_position(Order &order);

    void update_floating_risk(const Order &order);

    double floating_risk_ = 0;
    std::vector<Order> orders_;
    std::vector<Position> positions_;

    std::reference_wrapper<EventQueue> event_queue_;
    std::reference_wrapper<Portfolio> portfolio_;
    std::reference_wrapper<const MarketData> last_market_data_;
};
