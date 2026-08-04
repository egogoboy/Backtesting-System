#include "backtester/enums/Direction.hpp"
#include "backtester/enums/FillAction.hpp"
#include "backtester/events/FillEvent.hpp"
#include "backtester/models/Position.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include "test_utils.hpp"
#include <cstdlib>
#include <gtest/gtest.h>

const PortfolioConfig CONFIG(1, 10000);

TEST(Portfolio, CalculateFloatingLosses) {
    std::shared_ptr<Position> position_long_1 =
        std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::LONG, 1.175);
    std::shared_ptr<Position> position_long_2 =
        std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::LONG, 1.172);
    std::shared_ptr<Position> position_short =
        std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::SHORT, 1.177);

    Portfolio portfolio(CONFIG);

    portfolio.on_fill_event(std::make_shared<FillEvent>(position_long_1, FillAction::OPEN));
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_long_2, FillAction::OPEN));
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_short, FillAction::OPEN));
    EXPECT_LE(std::abs(portfolio.get_floating_losses(1.170) - 700), 10e-7);

    position_short->close_position(1.170);
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_short, FillAction::CLOSE));
    EXPECT_LE(std::abs(portfolio.get_floating_losses(1.170) - 700), 10e-7);

    position_long_2->close_position(1.170);
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_long_2, FillAction::CLOSE));
    EXPECT_LE(std::abs(portfolio.get_floating_losses(1.170) - 500), 10e-7);
}

TEST(Portfolio, CalculateBalanceAndRealizedPnL) {
    Portfolio portfolio(CONFIG);

    std::shared_ptr<Position> position_long =
        std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::LONG, 1.175);
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_long, FillAction::OPEN));

    EXPECT_LE(std::abs(portfolio.get_account_balance() - 10000), 10e-7);
    EXPECT_LE(std::abs(portfolio.get_floating_losses(1.170) - 500), 10e-7);

    position_long->close_position(1.170);
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_long, FillAction::CLOSE));

    EXPECT_LE(std::abs(portfolio.get_realized_pnl() + 500), 10e-7);
    EXPECT_LE(std::abs(portfolio.get_account_balance() - 9500), 10e-7);

    std::shared_ptr<Position> position_short =
        std::make_shared<Position>(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::SHORT, 1.175);
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_short, FillAction::OPEN));

    position_short->close_position(1.170);
    portfolio.on_fill_event(std::make_shared<FillEvent>(position_short, FillAction::CLOSE));

    EXPECT_LE(std::abs(portfolio.get_realized_pnl()), 10e-7);
    EXPECT_LE(std::abs(portfolio.get_account_balance() - 10000), 10e-7);
}

TEST(Portfolio, ChargeFee) {
    Portfolio portfolio(CONFIG);

    EXPECT_FALSE(portfolio.charge_fee(11000));
    EXPECT_TRUE(portfolio.charge_fee(100));

    EXPECT_EQ(portfolio.get_account_balance(), 9900);
    EXPECT_EQ(portfolio.get_realized_pnl(), 0);
}

TEST(Portfolio, ReserveFunds) {
    Portfolio portfolio(CONFIG);

    EXPECT_FALSE(portfolio.reserve_margin(11000));
    EXPECT_TRUE(portfolio.reserve_margin(100));

    EXPECT_EQ(portfolio.get_account_balance(), 10000);

    EXPECT_FALSE(portfolio.release_margin(101));
    EXPECT_TRUE(portfolio.release_margin(100));

    EXPECT_EQ(portfolio.get_account_balance(), 10000);
}

TEST(Portfolio, HasAvailableFunds) {
    Portfolio portfolio(CONFIG);

    EXPECT_TRUE(portfolio.reserve_margin(100));

    EXPECT_FALSE(portfolio.has_available_funds(10000));

    EXPECT_TRUE(portfolio.has_available_funds(9800));

    EXPECT_TRUE(portfolio.release_margin(100));

    EXPECT_TRUE(portfolio.has_available_funds(10000));
}
