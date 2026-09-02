#include "backtester/metric_calculator/MetricCalculator.hpp"
#include "backtester/enums/Direction.hpp"
#include "backtester/enums/FillAction.hpp"
#include "backtester/models/Order.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include "backtester/utils/PnLCalculator.hpp"
#include <algorithm>
#include <cstdlib>

MetricCalculator::MetricCalculator(const Portfolio &portfolio)
    : portfolio_{portfolio}, max_drawdown_{portfolio.get_total_equity()} {}

void MetricCalculator::on_market_event(const std::shared_ptr<MarketEvent> &event) {
    update_maximum_drawdown();
}

void MetricCalculator::on_fill_event(const std::shared_ptr<FillEvent> &event) {
    if (event->get_action() == FillAction::OPEN) {
        update_total_amount_of_trades();
    }

    if (event->get_action() == FillAction::CLOSE) {
        const Position &position = *(event->get_data());

        update_win_and_loss(position);

        update_r_multipliers_sum(position);
    }
}

Metrics MetricCalculator::calculate_metrics() const {
    Metrics metrics{};

    metrics.maximum_drawdown = max_drawdown_;

    if (total_amount_of_trades_ == 0) {
        return metrics;
    }

    metrics.win_rate = static_cast<double>(amount_of_winning_trades_) / total_amount_of_trades_;

    metrics.profit_factor = gross_profit_ / std::abs(gross_loss_);

    double win_rate = static_cast<double>(amount_of_winning_trades_) / total_amount_of_trades_;

    double loss_rate = static_cast<double>(amount_of_loss_trades_) / total_amount_of_trades_;

    double average_win = gross_profit_ / amount_of_winning_trades_;

    double average_loss = gross_loss_ / amount_of_loss_trades_;

    metrics.expectancy_money = (win_rate - average_win) / (loss_rate - average_loss);

    metrics.expectancy_r = r_multipliers_sum_ / total_amount_of_trades_;

    return metrics;
}

void MetricCalculator::update_total_amount_of_trades() {
    ++total_amount_of_trades_;
}

void MetricCalculator::update_maximum_drawdown() {
    max_drawdown_ = std::min(portfolio_.get().get_total_equity(), max_drawdown_);
}

void MetricCalculator::update_win_and_loss(const Position &position) {
    double position_pnl = PnLCalculator::get_position_realized_pnl(position);

    if (position_pnl > 0) {
        ++amount_of_winning_trades_;
        gross_profit_ += position_pnl;
    } else {
        ++amount_of_loss_trades_;
        gross_loss_ -= position_pnl;
    }
}

void MetricCalculator::update_r_multipliers_sum(const Position &position) {
    r_multipliers_sum_ += calculate_r_multiplier(position);
}

double MetricCalculator::calculate_r_multiplier(const Position &position) {
    double entry_price = position.get_entry_price();
    double stop_loss = position.get_stop_loss_order().lock()->get_trigger_price().value();
    double price_change = position.get_exit_price().value() - entry_price;

    if (position.get_direction() == Direction::SHORT) {
        price_change *= -1;
    }

    return price_change / (entry_price - stop_loss);
}
