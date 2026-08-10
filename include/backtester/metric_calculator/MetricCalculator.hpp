#pragma once

#include "backtester/events/FillEvent.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include <functional>
#include <memory>

class MetricCalculator {
  public:
    MetricCalculator(const Portfolio &portfolio);

    void on_market_event(const std::shared_ptr<MarketEvent> &event);

    void on_fill_event(const std::shared_ptr<FillEvent> &event);

  private:
    std::reference_wrapper<const Portfolio> portfolio_;

    double max_drawdown_;
};
