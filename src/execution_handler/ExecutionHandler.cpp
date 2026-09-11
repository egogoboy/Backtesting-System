#include "backtester/execution_handler/ExecutionHandler.hpp"
#include "backtester/enums/Direction.hpp"
#include "backtester/enums/FillAction.hpp"
#include "backtester/enums/OrderStatus.hpp"
#include "backtester/enums/OrderType.hpp"
#include "backtester/events/FillEvent.hpp"
#include <algorithm>
#include <memory>
#include <vector>

ExecutionHandler::ExecutionHandler(const ExecutionConfig &config, EventQueue &event_queue,
                                   Portfolio &portfolio, const MarketData &initial_market_data)
    : config_{config}, event_queue_{event_queue}, portfolio_{portfolio},
      last_market_data_{initial_market_data} {
    rng_ = std::mt19937_64(SPREAD_SEED);
}

void ExecutionHandler::on_market_event(const std::shared_ptr<MarketEvent> &event) {
    orders_to_execute_.clear();

    const MarketData &market_data = event->get_data();

    update_atr(market_data);

    last_market_data_ = market_data;

    if (portfolio_.get().get_total_equity() < maintenance_margin_) {
        execute_stop_out_liquidation();
        return;
    }

    std::erase_if(orders_, [&](const auto &order_ptr) {
        if (order_ptr->get_status() == OrderStatus::PENDING) {
            return handle_order_execution(order_ptr);
        }

        return true;
    });

    for (auto &order_ptr : orders_to_execute_) {
        execute_order(*order_ptr);
    }
}

void ExecutionHandler::on_order_event(const std::shared_ptr<OrderEvent> &event) {
    auto order = event->get_data();

    double current_price = last_market_data_.get_close();
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

bool ExecutionHandler::handle_order_execution(const std::shared_ptr<Order> &order) {
    if (!can_execute_order(*order, last_market_data_)) {
        return false;
    }

    if (order->is_exit_order() ||
        portfolio_.get().has_available_funds(calculate_order_required_margin(*order))) {
        orders_to_execute_.emplace_back(order);
        return true;
    }

    order->cancel();
    update_floating_risk(*order);
    return true;
}

void ExecutionHandler::execute_order(Order &order) {
    if (order.get_status() == OrderStatus::CANCELED) {
        return;
    }

    double slippage = calculate_spread();

    if (order.get_type() == OrderType::STOP) {
        order.convert_to_market();
    }

    if (order.get_direction() == Direction::SHORT) {
        slippage *= -1;
    }

    double entry_price = 0;
    if (order.get_type() == OrderType::MARKET) {
        entry_price = last_market_data_.get_close() + slippage;
    } else {
        entry_price = order.get_trigger_price().value();
    }

    try {
        fill_position(order, entry_price);

        order.execute();

        update_floating_risk(order);
    } catch (const std::logic_error &er) {
        // Failed to create Order. No actions needed
    }
}

bool ExecutionHandler::can_execute_order(const Order &order, const MarketData &market_data) {
    Direction direction = order.get_direction();

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

void ExecutionHandler::execute_stop_out_liquidation() {
    for (const auto &position : positions_) {
        position->close_position(last_market_data_.get_close());

        double margin = position->get_quantity() * position->get_instrument().get_contract_size() *
                        config_.margin_rate;
        portfolio_.get().release_margin(margin);

        update_maintenance_margin(-margin);

        event_queue_.get().push(std::make_shared<FillEvent>(position, FillAction::CLOSE));
    }

    positions_.clear();

    for (const auto &order : orders_) {
        order->cancel();
        update_floating_risk(*order);
    }
}

void ExecutionHandler::fill_position(Order &order, double target_price) {
    double margin =
        order.get_volume() * order.get_instrument().get_contract_size() * config_.margin_rate;

    if (order.get_role() == OrderRole::ENTRY) {
        auto position = std::make_shared<Position>(order.get_instrument(), order.get_volume(),
                                                   order.get_direction(), target_price);

        auto sl_order = std::make_shared<Order>(
            Order::make_stop_loss(position, order.get_stop_loss_price().value()));
        auto tp_order = std::make_shared<Order>(
            Order::make_take_profit(position, order.get_take_profit_price().value()));

        if (order.get_direction() == Direction::LONG) {
            orders_.emplace_back(tp_order);
            orders_.emplace_back(sl_order);
        } else {
            orders_.emplace_back(sl_order);
            orders_.emplace_back(tp_order);
        }

        position->set_stop_loss_order(sl_order);
        position->set_take_profit_order(tp_order);

        portfolio_.get().reserve_margin(margin);
        update_maintenance_margin(margin);

        positions_.emplace_back(position);

        order.assign_position(position);

        event_queue_.get().push(std::make_shared<FillEvent>(position, FillAction::OPEN));
    } else {
        std::shared_ptr<Position> position = order.get_position().lock();

        position->get_take_profit_order().lock()->cancel();
        position->get_stop_loss_order().lock()->cancel();

        position->close_position(target_price);

        positions_.erase(std::find_if(positions_.begin(), positions_.end(),
                                      [position](const std::shared_ptr<Position> &iterator) {
                                          return position.get() == iterator.get();
                                      }));

        portfolio_.get().release_margin(margin);
        update_maintenance_margin(-margin);

        event_queue_.get().push(std::make_shared<FillEvent>(position, FillAction::CLOSE));
    }
}

void ExecutionHandler::update_floating_risk(const Order &order) {
    double contract_size = order.get_instrument().get_contract_size();
    double volume = order.get_volume();

    double trigger_price = 0.0;
    if (order.get_type() == OrderType::MARKET) {
        trigger_price = last_market_data_.get_close();
    } else {
        trigger_price = order.get_trigger_price().value();
    }

    if (order.get_role() == OrderRole::ENTRY) {
        if (order.get_status() == OrderStatus::PENDING) {
            floating_risk_ += std::abs((trigger_price - order.get_stop_loss_price().value())) *
                              contract_size * volume;
        } else if (order.get_status() == OrderStatus::CANCELED) {
            floating_risk_ -= std::abs((trigger_price - order.get_stop_loss_price().value())) *
                              contract_size * volume;
        } else {
            floating_risk_ -=
                std::abs(order.get_position().lock()->get_entry_price() - trigger_price) *
                contract_size * volume;
        }
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
    double previous_close = last_market_data_.get_close();

    return std::max(std::max(current_high - current_low, std::abs(current_high - previous_close)),
                    std::abs(current_low - previous_close));
}

void ExecutionHandler::update_maintenance_margin(double margin) {
    maintenance_margin_ += margin * config_.maintenance_margin_rate;
}

double ExecutionHandler::calculate_order_required_margin(const Order &order) const {
    return order.get_volume() * order.get_instrument().get_contract_size() * config_.margin_rate;
}
