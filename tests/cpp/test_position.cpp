#include "backtester/enums/PositionStatus.hpp"
#include "backtester/models/Position.hpp"
#include "test_utils.hpp"
#include <gtest/gtest.h>

TEST(Position, Status) {
    Position position(GLOBAL_EURUSD_INSTRUMENT, 1, Direction::LONG, 1.178);
    EXPECT_EQ(position.get_status(), PositionStatus::OPEN);

    EXPECT_EQ(position.close_position(1.175), true);
    EXPECT_EQ(position.get_status(), PositionStatus::CLOSE);
}
