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
#include <memory>

const int INITIAL_PORTFOLIO_MONEY = 10000;

const ExecutionConfig EXECUTION_CONFIG(0.1, 0.05);

const PortfolioConfig PORTFOLIO_CONFIG(1, INITIAL_PORTFOLIO_MONEY);

const MarketData INITIAL_MARKET_DATA(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.175, 1.169, 1.170}, 0);

TEST(ExecutionHandler, ValidOrdersCreationAndFloatingRiskCalculation) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);

    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

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
    Portfolio portfolio(PORTFOLIO_CONFIG);

    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    execution_handler.on_order_event(std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.171, 1.168, 1.180))));

    EXPECT_LE(execution_handler.get_floating_risk(), 1e-7);

    execution_handler.on_order_event(std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.169, 1.169, 1.180))));

    EXPECT_LE(execution_handler.get_floating_risk(), 1e-7);
}

TEST(ExecutionHandler, ExecuteOrder) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);

    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

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

    next_market_data = MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.182, 1.170, 1.179}, 2);

    execution_handler.on_market_event(std::make_shared<MarketEvent>(next_market_data));

    EXPECT_TRUE(portfolio.has_available_funds(10000));
}

TEST(ExecutionHandler, StopOut) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);

    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    auto first_order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.0, 0.7, 1.180));

    execution_handler.on_order_event(std::make_shared<OrderEvent>(first_order));

    auto market_event = std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.171, 1.155, 1.156}, 1));

    portfolio.on_market_event(market_event);
    execution_handler.on_market_event(market_event);

    portfolio.on_fill_event(std::dynamic_pointer_cast<FillEvent>(event_queue.front()));
    event_queue.pop();

    market_event = std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.156, 1.158, 1.130, 1.132}, 2));

    portfolio.on_market_event(market_event);
    execution_handler.on_market_event(market_event);

    market_event = std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.129, 1.131, 1.100, 1.102}, 3));

    portfolio.on_market_event(market_event);
    execution_handler.on_market_event(market_event);

    market_event = std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.102, 1.105, 1.050, 1.055}, 4));

    portfolio.on_market_event(market_event);
    execution_handler.on_market_event(market_event);

    // Execution Handler has to send close fill event after stop out
    EXPECT_FALSE(event_queue.empty());

    EXPECT_EQ(event_queue.front()->get_type(), EventType::FILL);

    auto event = std::dynamic_pointer_cast<FillEvent>(event_queue.front());

    EXPECT_EQ(event->get_action(), FillAction::CLOSE);
}
