#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/config/RiskManagerConfig.hpp"
#include "backtester/core/EventQueue.hpp"
#include "backtester/enums/Direction.hpp"
#include "backtester/events/OrderEvent.hpp"
#include "backtester/events/SignalEvent.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Signal.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include "backtester/risk_manager/RiskManager.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>
#include <memory>

const PortfolioConfig PORTFOLIO_CONFIG(1, 10000);
const RiskManagerConfig RISK_MANAGER_CONFIG(0.01, 0.05, 0.3);

TEST(RiskManager, ProduceCorrectOrders) {
    EventQueue queue;

    Portfolio portfolio(PORTFOLIO_CONFIG);

    RiskManager risk_manager(RISK_MANAGER_CONFIG, queue, portfolio,
                             MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.175, 1.168, 1.172}, 0));

    risk_manager.on_signal_event(std::make_shared<SignalEvent>(
        Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.1715, 1.177)));

    std::shared_ptr<OrderEvent> event;

    EXPECT_TRUE(!queue.empty());

    event = std::dynamic_pointer_cast<OrderEvent>(queue.front());
    EXPECT_EQ(event->get_data()->get_direction(), Direction::LONG);
    EXPECT_EQ(event->get_data()->get_type(), OrderType::MARKET);
    EXPECT_LE(event->get_data()->get_volume() - 2, 1e-7);

    queue.pop();

    risk_manager.on_signal_event(std::make_shared<SignalEvent>(
        Signal::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.173, 1.1725, 1.175)));

    EXPECT_TRUE(!queue.empty());

    event = std::dynamic_pointer_cast<OrderEvent>(queue.front());
    EXPECT_EQ(event->get_data()->get_direction(), Direction::LONG);
    EXPECT_EQ(event->get_data()->get_type(), OrderType::LIMIT);
    EXPECT_LE(event->get_data()->get_volume() - 2, 1e-7);

    queue.pop();

    risk_manager.on_signal_event(std::make_shared<SignalEvent>(
        Signal::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.171, 1.170, 1.175)));

    event = std::dynamic_pointer_cast<OrderEvent>(queue.front());
    EXPECT_EQ(event->get_data()->get_direction(), Direction::LONG);
    EXPECT_EQ(event->get_data()->get_type(), OrderType::LIMIT);
    EXPECT_LE(event->get_data()->get_volume() - 1, 1e-7);

    EXPECT_TRUE(!queue.empty());
}

TEST(RiskManager, InvalidStopLossTakeProfit) {
    EventQueue queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);

    RiskManager risk_manager(RISK_MANAGER_CONFIG, queue, portfolio,
                             MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.175, 1.168, 1.172}, 0));

    risk_manager.on_signal_event(std::make_shared<SignalEvent>(
        Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.180, 1.160)));

    EXPECT_TRUE(queue.empty());
}

TEST(RiskManager, InvalidStopLossTakeProfitRatio) {
    EventQueue queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);

    RiskManager risk_manager(RISK_MANAGER_CONFIG, queue, portfolio,
                             MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.175, 1.168, 1.172}, 0));

    risk_manager.on_signal_event(std::make_shared<SignalEvent>(
        Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.160, 1.180)));

    EXPECT_TRUE(queue.empty());
}
