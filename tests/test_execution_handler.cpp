#include "backtester/config/ExecutionConfig.hpp"
#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/core/EventQueue.hpp"
#include "backtester/enums/EventType.hpp"
#include "backtester/enums/OrderStatus.hpp"
#include "backtester/events/OrderEvent.hpp"
#include "backtester/execution_handler/ExecutionHandler.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

ExecutionConfig conf(0.1, 0.05);

TEST(ExecutionHandler, ValidOrdersCreationAndFloatingRiskCalculation) {
    EventQueue event_queue;
    PortfolioConfig portfolio_config(1, 10000);
    Portfolio portfolio(portfolio_config);
    MarketData initial_market_data(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.175, 1.169, 1.170}, 0);

    ExecutionHandler execution_handler(conf, event_queue, portfolio, initial_market_data);

    auto event = std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.169, 1.180)));

    execution_handler.on_order_event(event);

    EXPECT_LE(execution_handler.get_floating_risk() - 100, 1e-7);

    event = std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.169, 1.168, 1.180)));

    execution_handler.on_order_event(event);

    EXPECT_LE(execution_handler.get_floating_risk() - 200, 1e-7);

    event = std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.171, 1.169, 1.180)));

    execution_handler.on_order_event(event);

    EXPECT_LE(execution_handler.get_floating_risk() - 400, 1e-7);
}

TEST(ExecutionHandler, InvalidOrdersCreation) {
    EventQueue event_queue;
    PortfolioConfig portfolio_config(1, 10000);
    Portfolio portfolio(portfolio_config);
    MarketData initial_market_data(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.175, 1.169, 1.170}, 0);

    ExecutionHandler execution_handler(conf, event_queue, portfolio, initial_market_data);

    execution_handler.on_order_event(std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.171, 1.168, 1.180))));

    EXPECT_LE(execution_handler.get_floating_risk(), 1e-7);

    execution_handler.on_order_event(std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.169, 1.169, 1.180))));

    EXPECT_LE(execution_handler.get_floating_risk(), 1e-7);
}

TEST(ExecutionHandler, ExecuteOrder) {
    EventQueue event_queue;
    PortfolioConfig portfolio_config(1, 10000);
    Portfolio portfolio(portfolio_config);
    MarketData initial_market_data(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.175, 1.169, 1.170}, 0);

    ExecutionHandler execution_handler(conf, event_queue, portfolio, initial_market_data);

    auto order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 0.1, 1.169, 1.180));

    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    MarketData next_market_data(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.174, 1.168, 1.172}, 1);

    execution_handler.on_market_event(std::make_shared<MarketEvent>(next_market_data));

    EXPECT_EQ(order->get_status(), OrderStatus::EXECUTED);

    EXPECT_FALSE(event_queue.empty());

    EXPECT_EQ(event_queue.front()->get_type(), EventType::FILL);

    EXPECT_FALSE(portfolio.has_available_funds(10000));

    EXPECT_TRUE(portfolio.has_available_funds(9000));
}
