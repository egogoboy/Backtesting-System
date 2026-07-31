#include "backtester/execution_handler/ExecutionHandler.hpp"
#include "backtester/enums/Direction.hpp"
#include "backtester/enums/FillAction.hpp"
#include "backtester/enums/OrderStatus.hpp"
#include "backtester/enums/OrderType.hpp"
#include "backtester/events/FillEvent.hpp"
#include <algorithm>
#include <memory>

ExecutionHandler::ExecutionHandler(const ExecutionConfig &config, EventQueue &event_queue,
                                   Portfolio &portfolio, const MarketData &initial_market_data)
    : config_{config}, event_queue_{event_queue}, portfolio_{portfolio},
      last_market_data_{initial_market_data} {
    rng_ = std::mt19937_64(SPREAD_SEED);
}

void ExecutionHandler::on_market_event(const std::shared_ptr<MarketEvent> &event) {
    const MarketData &market_data = event->get_data();

    update_atr(market_data);

    for (auto it = orders_.begin(); it != orders_.end();) {
        if ((*it)->get_status() == OrderStatus::PENDING) {
            if (can_execute_order(**it, market_data)) {
                execute_order(**it);
                it = orders_.erase(it);
            } else {
                ++it;
            }
        } else {
            it = orders_.erase(it);
        }
    }
}

void ExecutionHandler::on_order_event(const std::shared_ptr<OrderEvent> &event) {
    auto order = event->get_data();

    double current_price = last_market_data_.get().get_close();
    Direction direction = order->get_direction();

    if (order->get_type() == OrderType::LIMIT) {
        double limit_price = order->get_trigger_price().value();

        if ((direction == Direction::LONG && limit_price > current_price) ||
            (direction == Direction::SHORT && limit_price < current_price)) {
            return;
        }
    } else if (order->get_type() == OrderType::STOP) {
        double stop_price = order->get_trigger_price().value();

        if ((direction == Direction::LONG && stop_price <= current_price) ||
            (direction == Direction::SHORT && stop_price >= current_price)) {
            return;
        }
    }

    update_floating_risk(*order);
    orders_.emplace_back(order);
}

double ExecutionHandler::get_floating_risk() const {
    return floating_risk_;
}

void ExecutionHandler::execute_order(Order &order) {}

bool ExecutionHandler::can_execute_order(const Order &order, const MarketData &market_data) {
    Direction direction = order.get_direction();

    if (!portfolio_.get().has_available_funds(order.get_volume() *
                                              order.get_instrument().get_contract_size() *
                                              config_.margin_rate)) {
        return false;
    }

    switch (order.get_type()) {
    case OrderType::MARKET:
        return true;
    case OrderType::LIMIT:
        if (direction == Direction::LONG) {
            return order.get_trigger_price().value() >= market_data.get_low();
        }

        return order.get_trigger_price().value() <= market_data.get_high();
    case OrderType::STOP:
        if (direction == Direction::LONG) {
            return order.get_trigger_price().value() <= market_data.get_high();
        }

        return order.get_trigger_price().value() >= market_data.get_low();
    }

    return false;
}

void ExecutionHandler::fill_position(Order &order) {}

void ExecutionHandler::update_floating_risk(const Order &order) {
    double contract_size = order.get_instrument().get_contract_size();
    double volume = order.get_volume();

    double trigger_price = 0.0;
    if (order.get_type() == OrderType::MARKET) {
        trigger_price = last_market_data_.get().get_close();
    } else {
        trigger_price = order.get_trigger_price().value();
    }

    if (order.get_status() == OrderStatus::PENDING) {
        floating_risk_ += std::abs((trigger_price - order.get_stop_loss_price().value())) *
                          contract_size * volume;
    } else {
        floating_risk_ -= std::abs(order.get_position().lock()->get_entry_price() - trigger_price) *
                          contract_size * volume;
    }
}

double ExecutionHandler::calculate_spread() {
    return atr_ * spread_dist_(rng_);
}

void ExecutionHandler::update_atr(const MarketData &market_data) {
    if (number_of_periods_ < ATR_PERIOD) {
        ++number_of_periods_;
        atr_ += calculate_true_range(market_data) / ATR_PERIOD;
    } else {
        atr_ += (calculate_true_range(market_data) - atr_) / ATR_PERIOD;
    }
}

double ExecutionHandler::calculate_true_range(const MarketData &market_data) {
    double current_high = market_data.get_high();
    double current_low = market_data.get_low();
    double previous_close = last_market_data_.get().get_close();

    return std::max(std::max(current_high - current_low, std::abs(current_high - previous_close)),
                    std::abs(current_low - previous_close));
}
