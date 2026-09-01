#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/enums/Direction.hpp"
#include "backtester/metric_calculator/MetricCalculator.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Order.hpp"
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

TEST(MetricCalculator, WinRate) {
    Portfolio portfolio(PORTFOLIO_CONFIG);

    MetricCalculator metric_calculator(portfolio);

    std::shared_ptr<Position> position =
        std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 0.1, Direction::LONG, 1.170);
    std::shared_ptr<Order> order = std::make_shared<Order>(Order::make_stop_loss(position, 1.160));
    position->set_stop_loss_order(order);
    metric_calculator.on_fill_event(std::make_shared<FillEvent>(position, FillAction::OPEN));

    position->close_position(1.165);
    metric_calculator.on_fill_event(std::make_shared<FillEvent>(position, FillAction::CLOSE));

    for (int i = 0; i < 2; ++i) {
        position =
            std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 0.1, Direction::LONG, 1.170);
        order = std::make_shared<Order>(Order::make_stop_loss(position, 1.160));
        position->set_stop_loss_order(order);
        metric_calculator.on_fill_event(std::make_shared<FillEvent>(position, FillAction::OPEN));

        position->close_position(1.175);
        metric_calculator.on_fill_event(std::make_shared<FillEvent>(position, FillAction::CLOSE));
    }

    EXPECT_DOUBLE_EQ(metric_calculator.calculate_metrics().win_rate, 0.66666666666666666);

    for (int i = 0; i < 2; ++i) {
        position =
            std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 0.1, Direction::LONG, 1.170);
        order = std::make_shared<Order>(Order::make_stop_loss(position, 1.160));
        position->set_stop_loss_order(order);
        metric_calculator.on_fill_event(std::make_shared<FillEvent>(position, FillAction::OPEN));

        position->close_position(1.165);
        metric_calculator.on_fill_event(std::make_shared<FillEvent>(position, FillAction::CLOSE));
    }

    EXPECT_DOUBLE_EQ(metric_calculator.calculate_metrics().win_rate, 0.4);
}
