#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/metric_calculator/MetricCalculator.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>
#include <memory>

const PortfolioConfig PORTFOLIO_CONFIG(1, 10000);

TEST(MetricCalculator, MaximumDrawdown) {
    Portfolio portfolio(PORTFOLIO_CONFIG);

    MetricCalculator metric_calculator(portfolio);

    portfolio.charge_fee(1000);

    MarketData market_data(GLOBAL_EURUSD_INSTRUMENT, {1.175, 1.177, 1.1735, 1.174}, 1);

    metric_calculator.on_market_event(std::make_shared<MarketEvent>(market_data));

    EXPECT_EQ(metric_calculator.calculate_metrics().maximum_drawdown, portfolio.get_total_equity());
}
