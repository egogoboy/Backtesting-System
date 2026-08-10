#include "backtester/metric_calculator/MetricCalculator.hpp"
#include <algorithm>

MetricCalculator::MetricCalculator(const Portfolio &portfolio)
    : portfolio_{portfolio}, max_drawdown_{portfolio.get_total_equity()} {}

void MetricCalculator::on_market_event(const std::shared_ptr<MarketEvent> &event) {
    update_maximum_drawdown();
}

void MetricCalculator::on_fill_event(const std::shared_ptr<FillEvent> &event) {}

void MetricCalculator::update_maximum_drawdown() {
    max_drawdown_ = std::min(portfolio_.get().get_total_equity(), max_drawdown_);
}
