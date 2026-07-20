#include "backtester/risk_manager/RiskManager.hpp"
#include "backtester/events/OrderEvent.hpp"
#include "backtester/models/Order.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include <memory>

RiskManager::RiskManager(const RiskManagerConfig &config, EventQueue &event_queue,
                         const Portfolio &portfolio, const MarketData &init_market_data)
    : event_queue_{event_queue}, portfolio_{portfolio}, last_market_data_{init_market_data},
      config_{config} {}

void RiskManager::on_signal_event(const std::shared_ptr<SignalEvent> &event) const {
    const Signal &signal = event->get_data();

    if (!check_stop_loss_take_profit(signal)) {
        return;
    }

    if (!check_stop_loss_take_profit_ratio(signal)) {
        return;
    }

    double safe_capital = calculate_safe_capital();

    if (!check_total_accout_risk(safe_capital)) {
        return;
    }

    place_order(create_order(signal, calculate_position_size(signal, safe_capital)));
}

void RiskManager::on_market_event(const std::shared_ptr<MarketEvent> &event) {
    last_market_data_ = event->get_data();
}

void RiskManager::place_order(const std::shared_ptr<Order> &order) const {
    event_queue_.get().push(std::make_shared<OrderEvent>(order));
}

std::shared_ptr<Order> RiskManager::create_order(const Signal &signal, double position_size) {
    switch (signal.get_type()) {
    case OrderType::MARKET:
        return std::make_shared<Order>(
            Order::make_market(signal.get_instrument(), signal.get_direction(), position_size,
                               signal.get_stop_loss_price(), signal.get_take_profit_price()));
    case OrderType::LIMIT:
        return std::make_shared<Order>(
            Order::make_limit(signal.get_instrument(), signal.get_direction(), position_size,
                              signal.get_entry_price().value(), signal.get_stop_loss_price(),
                              signal.get_take_profit_price()));
    case OrderType::STOP:
        return std::make_shared<Order>(
            Order::make_stop(signal.get_instrument(), signal.get_direction(), position_size,
                             signal.get_entry_price().value(), signal.get_stop_loss_price(),
                             signal.get_take_profit_price()));
    }
}

bool RiskManager::check_stop_loss_take_profit(const Signal &signal) const {
    bool stop_loss_validity = true;
    bool take_profit_validity = true;

    double entry_price = get_signal_entry_price(signal);

    switch (signal.get_direction()) {
    case Direction::LONG:
        stop_loss_validity = signal.get_stop_loss_price() < entry_price;
        break;
    case Direction::SHORT:
        stop_loss_validity = signal.get_stop_loss_price() > entry_price;
        break;
    }

    switch (signal.get_direction()) {
    case Direction::LONG:
        take_profit_validity = signal.get_take_profit_price() > entry_price;
        break;
    case Direction::SHORT:
        take_profit_validity = signal.get_take_profit_price() < entry_price;
        break;
    }

    return stop_loss_validity && take_profit_validity;
}

bool RiskManager::check_stop_loss_take_profit_ratio(const Signal &signal) const {
    double entry_price = get_signal_entry_price(signal);
    double stop_loss_price = signal.get_stop_loss_price();
    double take_profit_price = signal.get_take_profit_price();

    double sl_tp_ratio =
        std::abs((entry_price - stop_loss_price) / (entry_price - take_profit_price));

    return sl_tp_ratio <= config_.sl_tp_ratio;
}

bool RiskManager::check_total_accout_risk(double safe_capital) const {
    double account_balance = portfolio_.get().get_account_balance();

    return (account_balance - safe_capital) / account_balance < config_.total_account_risk;
}

// TODO
// Add Execution Handler pending orders risk to the formula
double RiskManager::calculate_safe_capital() const {
    return portfolio_.get().get_account_balance() -
           portfolio_.get().get_floating_losses(last_market_data_.get().get_close());
}

double RiskManager::calculate_position_size(const Signal &signal, double safe_capital) const {
    double safe_money = safe_capital * config_.risk_per_trade;

    return safe_money / std::abs(get_signal_entry_price(signal) - signal.get_stop_loss_price()) /
           signal.get_instrument().get_contract_size();
}

double RiskManager::get_signal_entry_price(const Signal &signal) const {
    if (signal.get_entry_price().has_value()) {
        return signal.get_entry_price().value();
    }

    return last_market_data_.get().get_close();
}
