#include "backtester/portfolio/Portfolio.hpp"
#include "backtester/enums/FillAction.hpp"
#include "backtester/utils/PnLCalculator.hpp"

#include <memory>

void Portfolio::on_fill_event(const std::shared_ptr<FillEvent> &event) {
    std::shared_ptr<const Position> position = event->get_data();

    if (event->get_action() == FillAction::OPEN) {
        opened_positions_.emplace_back(position);
    } else {
        auto opened_position = std::find_if(opened_positions_.begin(), opened_positions_.end(),
                                            [position](const std::shared_ptr<const Position> &ptr) {
                                                return position.get() == ptr.get();
                                            });

        if (opened_position != opened_positions_.end()) {
            closed_positions_.emplace_back(std::move(*opened_position));
            opened_positions_.erase(opened_position);
        }

        double pnl = PnLCalculator::get_position_realized_pnl(*position);

        account_balance_ += pnl;
        realized_pnl_ += pnl;
    }
}

void Portfolio::on_market_event(const std::shared_ptr<MarketEvent> &event) {
    unrealized_pnl_ = 0;

    for (const auto &position : opened_positions_) {
        unrealized_pnl_ +=
            PnLCalculator::get_position_unrealized_pnl(*position, event->get_data().get_close());
    }
}

bool Portfolio::charge_fee(double amount) {
    if (!has_available_funds(amount)) {
        return false;
    }

    account_balance_ -= amount;
    return true;
}

bool Portfolio::reserve_margin(double amount) {
    if (!has_available_funds(amount)) {
        return false;
    }

    reserved_funds_ += amount;

    return true;
}

bool Portfolio::release_margin(double amount) {
    if (amount > reserved_funds_) {
        return false;
    }

    reserved_funds_ -= amount;

    return true;
}

bool Portfolio::has_available_funds(double amount) const {
    return amount <= account_balance_ + unrealized_pnl_ - reserved_funds_;
}

double Portfolio::get_floating_losses(double current_price) const {
    double floating_losses = 0;

    for (const auto &position : opened_positions_) {
        double floating_pnl = PnLCalculator::get_position_unrealized_pnl(*position, current_price);

        floating_losses += std::max(-floating_pnl, 0.0);
    }

    return floating_losses;
}

double Portfolio::get_realized_pnl() const {
    return realized_pnl_;
}

double Portfolio::get_unrealized_pnl() const {
    return unrealized_pnl_;
}

double Portfolio::get_account_balance() const {
    return account_balance_;
}

double Portfolio::get_total_equity() const {
    return account_balance_ + unrealized_pnl_;
}
