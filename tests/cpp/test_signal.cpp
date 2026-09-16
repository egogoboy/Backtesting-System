#include "backtester/enums/Direction.hpp"
#include "backtester/enums/OrderType.hpp"
#include "backtester/models/Signal.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

TEST(Signal, Types) {
    Signal signal = Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1);
    EXPECT_EQ(signal.get_type(), OrderType::MARKET);

    signal = Signal::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1, 1);
    EXPECT_EQ(signal.get_type(), OrderType::LIMIT);

    signal = Signal::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG, 1, 1, 1);
    EXPECT_EQ(signal.get_type(), OrderType::STOP);
}
