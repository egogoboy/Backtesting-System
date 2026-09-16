#include "backtester/models/Position.hpp"
#include "backtester/utils/PnLCalculator.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

TEST(PnLCalculator, CalculateUnrealizedPnL) {
    Position position(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::LONG, 1.175);

    EXPECT_LE(std::abs(PnLCalculator::get_position_unrealized_pnl(position, 1.173) + 200), 10e-7);
}

TEST(PnLCalculator, CalculateRealizedPnL) {
    Position position(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::LONG, 1.175);

    position.close_position(1.177);

    EXPECT_LE(std::abs(PnLCalculator::get_position_realized_pnl(position) - 200), 10e-7);
}
