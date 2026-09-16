#include "backtester/config/ExecutionConfig.hpp"
#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/core/EventQueue.hpp"
#include "backtester/enums/Direction.hpp"
#include "backtester/enums/EventType.hpp"
#include "backtester/enums/FillAction.hpp"
#include "backtester/enums/OrderStatus.hpp"
#include "backtester/enums/OrderType.hpp"
#include "backtester/events/FillEvent.hpp"
#include "backtester/events/OrderEvent.hpp"
#include "backtester/execution_handler/ExecutionHandler.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Order.hpp"
#include "backtester/models/Position.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include "test_utils.hpp"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <memory>

namespace {

// Pops the front of the queue and casts it to a FillEvent. Returns nullptr if
// the queue is empty or the front event isn't a FillEvent. Used only by the
// tests added below the original four, which keep their existing inline style.
std::shared_ptr<FillEvent> pop_fill_event(EventQueue &queue) {
    if (queue.empty()) {
        return nullptr;
    }
    auto event = queue.front();
    queue.pop();
    return std::dynamic_pointer_cast<FillEvent>(event);
}

} // namespace

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

    // MARKET order: risk = |current_close(1.170) - sl(1.169)| * contract(100000) * volume(1)
    EXPECT_NEAR(execution_handler.get_floating_risk(), 100.0, 1e-7);

    event = std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.169, 1.168, 1.180)));

    execution_handler.on_order_event(event);

    // LIMIT order: risk uses the order's own trigger price, not current close.
    // += |1.169 - 1.168| * 100000 * 1 = 100
    EXPECT_NEAR(execution_handler.get_floating_risk(), 200.0, 1e-7);

    event = std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.171, 1.169, 1.180)));

    execution_handler.on_order_event(event);

    // STOP order: += |1.171 - 1.169| * 100000 * 1 = 200
    EXPECT_NEAR(execution_handler.get_floating_risk(), 400.0, 1e-7);
}

TEST(ExecutionHandler, InvalidOrdersCreation) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);

    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    // LIMIT LONG above current price (1.171 > 1.170) is rejected at submission.
    execution_handler.on_order_event(std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.171, 1.168, 1.180))));

    EXPECT_NEAR(execution_handler.get_floating_risk(), 0.0, 1e-7);

    // STOP LONG at/below current price (1.169 <= 1.170) is rejected at submission.
    execution_handler.on_order_event(std::make_shared<OrderEvent>(std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.169, 1.169, 1.180))));

    EXPECT_NEAR(execution_handler.get_floating_risk(), 0.0, 1e-7);
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

    // margin = 0.1 * 100000 * 0.1 = 1000
    EXPECT_FALSE(portfolio.has_available_funds(10000));

    EXPECT_TRUE(portfolio.has_available_funds(9000));

    next_market_data = MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.182, 1.170, 1.179}, 2);

    // high (1.182) reaches take-profit (1.180): position closes, margin released.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(next_market_data));

    EXPECT_TRUE(portfolio.has_available_funds(10000));
}

TEST(ExecutionHandler, StopOut) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);

    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    // Wide stop-loss (0.7) so the position stays open through the drawdown
    // below and gets closed by the stop-out itself, not by its own SL.
    auto first_order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.0, 0.7, 1.180));

    execution_handler.on_order_event(std::make_shared<OrderEvent>(first_order));

    // Portfolio::on_market_event is called alongside ExecutionHandler's, the
    // way the real event loop would dispatch a MarketEvent to both -- this is
    // what keeps unrealized_pnl_ (and therefore total_equity) in sync with
    // price, which is what the stop-out trigger now depends on.
    auto market_event = std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.171, 1.155, 1.156}, 1));

    portfolio.on_market_event(market_event);
    execution_handler.on_market_event(market_event);

    // Portfolio also needs to know about the newly-opened position so its
    // unrealized PnL calc has something to track -- the real event loop would
    // dispatch this FillEvent the moment it's pushed.
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

    // Exactly one position was open, so exactly one CLOSE fill should have
    // been pushed -- guards against orders_to_execute_ re-firing already-
    // executed orders on a later tick (the duplicate-execution bug).
    event_queue.pop();
    EXPECT_TRUE(event_queue.empty());
}

// ===========================================================================
// Additional coverage
// ===========================================================================

// ---------------------------------------------------------------------------
// Submission-time price gating: SHORT side (LONG side is covered by
// ValidOrdersCreationAndFloatingRiskCalculation / InvalidOrdersCreation above)
// ---------------------------------------------------------------------------

TEST(ExecutionHandler, LimitOrder_Short_RejectedWhenTriggerBelowCurrentPrice) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    auto order = std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT, 1, 1.169, 1.180, 1.160));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    EXPECT_NEAR(execution_handler.get_floating_risk(), 0.0, 1e-7);
    EXPECT_EQ(order->get_status(), OrderStatus::PENDING);
}

TEST(ExecutionHandler, LimitOrder_Short_AcceptedWhenTriggerAtOrAboveCurrentPrice) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    auto order = std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT, 1, 1.170, 1.180, 1.160));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    // += |1.170 - 1.180| * 100000 * 1 = 1000
    EXPECT_NEAR(execution_handler.get_floating_risk(), 1000.0, 1e-7);
}

TEST(ExecutionHandler, StopOrder_Short_RejectedWhenTriggerAtOrAboveCurrentPrice) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    auto order = std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT, 1, 1.170, 1.180, 1.160));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    EXPECT_NEAR(execution_handler.get_floating_risk(), 0.0, 1e-7);
}

TEST(ExecutionHandler, StopOrder_Short_AcceptedWhenTriggerBelowCurrentPrice) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    auto order = std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT, 1, 1.169, 1.180, 1.160));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    // += |1.169 - 1.180| * 100000 * 1 = 1100
    EXPECT_NEAR(execution_handler.get_floating_risk(), 1100.0, 1e-7);
}

// ---------------------------------------------------------------------------
// Fill-price mechanics
// ---------------------------------------------------------------------------

TEST(ExecutionHandler, LimitOrder_FillsAtExactTriggerPrice_RegardlessOfSlippage) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    auto order = std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.165, 1.160, 1.175));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    // Low doesn't reach the trigger yet.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.171, 1.166, 1.168}, 1)));
    EXPECT_EQ(order->get_status(), OrderStatus::PENDING);
    EXPECT_TRUE(event_queue.empty());

    // Low dips through the trigger; fill happens at the exact trigger price,
    // never at the tick's close, regardless of ATR/slippage.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.168, 1.168, 1.163, 1.164}, 2)));

    auto fill = pop_fill_event(event_queue);
    ASSERT_NE(fill, nullptr);
    EXPECT_DOUBLE_EQ(fill->get_data()->get_entry_price(), 1.165);
}

TEST(ExecutionHandler, StopOrder_ConvertsToMarketTypeOnExecution) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    auto order = std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.175, 1.165, 1.185));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));
    ASSERT_EQ(order->get_type(), OrderType::STOP);

    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.176, 1.171, 1.174}, 1)));

    EXPECT_EQ(order->get_status(), OrderStatus::EXECUTED);
    EXPECT_EQ(order->get_type(), OrderType::MARKET); // converted on execution
    EXPECT_FALSE(event_queue.empty());
}

TEST(ExecutionHandler, ShortMarketOrder_FullLifecycle_ClosesViaStopLoss) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    MarketData flat_initial(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 0);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio, flat_initial);

    // For a SHORT position, sl must sit above entry and tp below it.
    auto order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT, 0.1, 1.180, 1.160));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    // Flat tick: zero slippage, fills at exactly 1.170.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 1)));

    ASSERT_EQ(order->get_status(), OrderStatus::EXECUTED);
    auto open_fill = pop_fill_event(event_queue);
    ASSERT_NE(open_fill, nullptr);
    EXPECT_EQ(open_fill->get_data()->get_direction(), Direction::SHORT);
    EXPECT_DOUBLE_EQ(open_fill->get_data()->get_entry_price(), 1.170);
    EXPECT_FALSE(portfolio.has_available_funds(10000));
    EXPECT_TRUE(portfolio.has_available_funds(9000)); // margin = 0.1*100000*0.1 = 1000

    // Price rises through the stop-loss (1.180): the SHORT position's SL is a
    // STOP order in the LONG direction, triggered when high reaches it.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.175, 1.182, 1.174, 1.180}, 2)));

    auto close_fill = pop_fill_event(event_queue);
    ASSERT_NE(close_fill, nullptr);
    EXPECT_EQ(close_fill->get_action(), FillAction::CLOSE);
    EXPECT_EQ(close_fill->get_data()->get_status(), PositionStatus::CLOSE);
    EXPECT_TRUE(portfolio.has_available_funds(10000));
}

TEST(ExecutionHandler, TakeProfitOrder_Short_FillsAtExactTriggerPrice) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    MarketData flat_initial(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 0);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio, flat_initial);

    auto order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT, 0.1, 1.180, 1.160));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 1)));
    pop_fill_event(event_queue); // drain the OPEN fill

    // Low dips through the take-profit (1.160): TP is a LIMIT order in the
    // LONG direction, so it fills at the exact trigger price, not the close.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.165, 1.166, 1.158, 1.159}, 2)));

    auto close_fill = pop_fill_event(event_queue);
    ASSERT_NE(close_fill, nullptr);
    EXPECT_EQ(close_fill->get_action(), FillAction::CLOSE);
    EXPECT_DOUBLE_EQ(close_fill->get_data()->get_exit_price().value(), 1.160);
    EXPECT_TRUE(portfolio.has_available_funds(10000));
}

TEST(ExecutionHandler, StopOrder_ExecutesWithSlippageWithinConfiguredBounds) {
    // Drives a non-zero, but exactly known, ATR via two non-flat candles, then
    // triggers a STOP entry and confirms the fill price falls within the
    // config-defined spread bounds around the tick's close. No exact price is
    // asserted: the RNG draw itself is not guaranteed portable across
    // standard library implementations, only its *range* is.
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    MarketData initial(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 0);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio, initial);

    constexpr double MIN_SPREAD = 1e-4;
    constexpr double MAX_SPREAD = 5e-4;
    constexpr double ATR_PERIOD = 14.0;

    double prev_close = 1.170;
    double atr = 0.0;
    auto true_range = [](double high, double low, double prev_close) {
        return std::max({high - low, std::abs(high - prev_close), std::abs(low - prev_close)});
    };

    // Tick 1: warms up ATR, no orders pending yet.
    MarketData md1(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.174, 1.167, 1.172}, 1);
    atr += true_range(1.174, 1.167, prev_close) / ATR_PERIOD;
    prev_close = 1.172;
    execution_handler.on_market_event(std::make_shared<MarketEvent>(md1));

    auto order = std::make_shared<Order>(
        Order::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.175, 1.160, 1.190));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    // Tick 2: high reaches the trigger -> stop converts to market and fills.
    MarketData md2(GLOBAL_EURUSD_INSTRUMENT, {1.172, 1.178, 1.171, 1.176}, 2);
    atr += (true_range(1.178, 1.171, prev_close)) / ATR_PERIOD;
    execution_handler.on_market_event(std::make_shared<MarketEvent>(md2));

    ASSERT_EQ(order->get_status(), OrderStatus::EXECUTED);
    ASSERT_EQ(order->get_type(), OrderType::MARKET);

    auto fill = pop_fill_event(event_queue);
    ASSERT_NE(fill, nullptr);

    double close = 1.176;
    double lower_bound = close + (atr * MIN_SPREAD);
    double upper_bound = close + (atr * MAX_SPREAD);
    double entry_price = fill->get_data()->get_entry_price();

    EXPECT_GE(entry_price, lower_bound - 1e-9);
    EXPECT_LE(entry_price, upper_bound + 1e-9);
}

// ---------------------------------------------------------------------------
// Funds / margin
// ---------------------------------------------------------------------------

TEST(ExecutionHandler, InsufficientFunds_EntryOrderCanceledImmediately) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio,
                                       INITIAL_MARKET_DATA);

    // volume=2 => margin = 2 * 100000 * 0.1 = 20000 > 10000 balance.
    auto order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 2, 1.160, 1.180));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));
    // += |1.170 - 1.160| * 100000 * 2 = 2000
    EXPECT_NEAR(execution_handler.get_floating_risk(), 2000.0, 1e-7);

    // Flat vs. the initial close, so the submission-time and cancellation-time
    // trigger prices match exactly and the unwind nets to precisely zero.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 1)));

    // Price condition passed (MARKET always does), but funds didn't: the
    // order is canceled outright rather than left pending for retry.
    EXPECT_EQ(order->get_status(), OrderStatus::CANCELED);
    EXPECT_TRUE(event_queue.empty());
    EXPECT_NEAR(execution_handler.get_floating_risk(), 0.0, 1e-7);
}

TEST(ExecutionHandler, ExitOrders_ExecuteEvenWhenEntryConsumedAllMargin) {
    // Regression test: can_execute_order() no longer applies a funds check to
    // exit orders (handle_order_execution special-cases is_exit_order()), so
    // a stop-loss/take-profit can't be stranded behind its own entry's margin
    // reservation.
    EventQueue event_queue;
    PortfolioConfig tight_portfolio_config(1, 100); // == exactly one entry's margin
    Portfolio portfolio(tight_portfolio_config);
    MarketData flat_initial(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 0);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio, flat_initial);

    // margin = 0.01 * 100000 * 0.1 = 100 == full balance.
    auto order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 0.01, 1.160, 1.180));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 1)));

    ASSERT_EQ(order->get_status(), OrderStatus::EXECUTED);
    pop_fill_event(event_queue);                       // drain the OPEN fill
    ASSERT_FALSE(portfolio.has_available_funds(0.01)); // fully reserved

    // Price breaches the stop-loss: should still execute despite zero
    // available funds for a fresh entry-sized margin check.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.165, 1.166, 1.158, 1.159}, 2)));

    auto close_fill = pop_fill_event(event_queue);
    ASSERT_NE(close_fill, nullptr);
    EXPECT_EQ(close_fill->get_action(), FillAction::CLOSE);
    EXPECT_TRUE(portfolio.has_available_funds(100)); // margin released back
}

// ---------------------------------------------------------------------------
// Duplicate-execution regression (orders_to_execute_ must not persist
// already-filled orders across ticks)
// ---------------------------------------------------------------------------

TEST(ExecutionHandler, MultipleTicks_DoesNotReExecuteAlreadyFilledOrders) {
    EventQueue event_queue;
    Portfolio portfolio(PORTFOLIO_CONFIG);
    MarketData flat_initial(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 0);
    ExecutionHandler execution_handler(EXECUTION_CONFIG, event_queue, portfolio, flat_initial);

    auto order = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 0.1, 1.160, 1.180));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order));

    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 1)));

    ASSERT_EQ(order->get_status(), OrderStatus::EXECUTED);
    auto open_fill = pop_fill_event(event_queue);
    ASSERT_NE(open_fill, nullptr);
    EXPECT_TRUE(event_queue.empty()); // exactly one fill, nothing queued behind it

    // Feed several more ticks where price never reaches SL/TP. If a stale
    // entry lingered in orders_to_execute_, each of these would silently
    // re-run fill_position: a phantom OPEN fill, doubled margin reservation,
    // and floating_risk_ drifting further with every tick.
    for (uint64_t ts = 2; ts <= 4; ++ts) {
        execution_handler.on_market_event(std::make_shared<MarketEvent>(
            MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.171, 1.169, 1.170}, ts)));
        EXPECT_TRUE(event_queue.empty());
    }

    // Margin still reserved exactly once: 0.1 * 100000 * 0.1 = 1000.
    EXPECT_FALSE(portfolio.has_available_funds(9500));
    EXPECT_TRUE(portfolio.has_available_funds(9000));

    // += |1.170 - 1.160| * 100000 * 0.1 = 100, unaffected by the later ticks.
    EXPECT_NEAR(execution_handler.get_floating_risk(), 100.0, 1e-7);
}

// ---------------------------------------------------------------------------
// Stop-out: pending entry order gets canceled and its floating risk unwound
// ---------------------------------------------------------------------------

TEST(ExecutionHandler, StopOut_CancelsPendingEntryOrdersAndUnwindsTheirFloatingRisk) {
    EventQueue event_queue;
    // Exaggerated maintenance_margin_rate so a single filled position pushes
    // maintenance_margin_ above total_equity, forcing a stop-out on the tick
    // right after it opens. Never calling Portfolio::on_market_event keeps
    // unrealized_pnl_ (and therefore total_equity) pinned to account_balance.
    ExecutionConfig stop_out_conf(/*margin_rate=*/0.1, /*maintenance_margin_rate=*/1.5);
    PortfolioConfig portfolio_config(1, 10000);
    Portfolio portfolio(portfolio_config);
    MarketData flat_initial(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 0);
    ExecutionHandler execution_handler(stop_out_conf, event_queue, portfolio, flat_initial);

    // Order A: executes immediately, consumes the whole balance as margin.
    auto order_a = std::make_shared<Order>(
        Order::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1.0, 1.160, 1.180));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order_a));

    // Order B: a resting limit far from price, still pending when the
    // stop-out fires. sl/tp are irrelevant to triggering it, only to the
    // floating-risk math once it's created.
    auto order_b = std::make_shared<Order>(
        Order::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1.100, 1.090, 1.150));
    execution_handler.on_order_event(std::make_shared<OrderEvent>(order_b));

    // Submission risk: A (MARKET) uses current close 1.170 -> |1.170-1.160|*100000*1 = 1000
    //                  B (LIMIT) uses its own trigger 1.100 -> |1.100-1.090|*100000*1 = 1000
    EXPECT_NEAR(execution_handler.get_floating_risk(), 2000.0, 1e-7);

    // Tick 1, flat: A executes at exactly 1.170 (zero slippage); B's trigger
    // (1.100) is nowhere near this tick's low (1.170), so it stays pending.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 1)));

    ASSERT_EQ(order_a->get_status(), OrderStatus::EXECUTED);
    EXPECT_EQ(order_b->get_status(), OrderStatus::PENDING);

    auto open_fill = pop_fill_event(event_queue);
    ASSERT_NE(open_fill, nullptr);
    auto position = open_fill->get_data();
    EXPECT_FALSE(portfolio.has_available_funds(0.01)); // fully reserved (margin = 10000)

    // A's execution at the same flat price nets zero change to its own risk
    // contribution, so the total is still 2000 (1000 from A + 1000 from B).
    EXPECT_NEAR(execution_handler.get_floating_risk(), 2000.0, 1e-7);

    // Tick 2, flat: maintenance_margin_ (10000 * 1.5 = 15000) now exceeds
    // total_equity (10000) -> stop-out fires before any order processing.
    execution_handler.on_market_event(std::make_shared<MarketEvent>(
        MarketData(GLOBAL_EURUSD_INSTRUMENT, {1.170, 1.170, 1.170, 1.170}, 2)));

    // Position A force-closed at this tick's close, margin released.
    EXPECT_EQ(position->get_status(), PositionStatus::CLOSE);
    ASSERT_TRUE(position->get_exit_price().has_value());
    EXPECT_DOUBLE_EQ(position->get_exit_price().value(), 1.170);
    EXPECT_TRUE(portfolio.has_available_funds(10000));

    // Order B (still ENTRY/PENDING) gets canceled by the stop-out, unwinding
    // its 1000 contribution. A's SL/TP (EXIT-role) orders also get canceled,
    // but exit orders never touch floating_risk_ in the first place, so only
    // B's cancellation should move the number: 2000 - 1000 = 1000.
    EXPECT_EQ(order_b->get_status(), OrderStatus::CANCELED);
    EXPECT_NEAR(execution_handler.get_floating_risk(), 1000.0, 1e-7);

    // Exactly one CLOSE fill for the forced liquidation, nothing else --
    // canceling B produces no FillEvent since it never opened a position.
    auto close_fill = pop_fill_event(event_queue);
    ASSERT_NE(close_fill, nullptr);
    EXPECT_EQ(close_fill->get_action(), FillAction::CLOSE);
    EXPECT_TRUE(event_queue.empty());
}
