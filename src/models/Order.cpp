#include "backtester/models/Order.hpp"
#include "backtester/models/Position.hpp"

Order Order::make_market(const Instrument &instrument, Direction direction, double volume,
                         double stop_loss_price, double take_profit_price) {
    return Order(instrument, direction, OrderRole::ENTRY, volume, OrderType::MARKET, std::nullopt,
                 stop_loss_price, take_profit_price, nullptr);
}

Order Order::make_limit(const Instrument &instrument, Direction direction, double volume,
                        double trigger_price, double stop_loss_price, double take_profit_price) {
    return Order(instrument, direction, OrderRole::ENTRY, volume, OrderType::LIMIT, trigger_price,
                 stop_loss_price, take_profit_price, nullptr);
}

Order Order::make_stop(const Instrument &instrument, Direction direction, double volume,
                       double trigger_price, double stop_loss_price, double take_profit_price) {
    return Order(instrument, direction, OrderRole::ENTRY, volume, OrderType::STOP, trigger_price,
                 stop_loss_price, take_profit_price, nullptr);
}

Order Order::make_stop_loss(const std::shared_ptr<Position> &position, double trigger_price) {
    Direction order_direction = opposite(position->get_direction());

    if ((position->get_direction() == Direction::SHORT &&
         trigger_price <= position->get_entry_price()) ||
        (position->get_direction() == Direction::LONG &&
         trigger_price >= position->get_entry_price())) {
        throw std::logic_error("Invalid entry price for stop loss order");
    }

    return Order(position->get_instrument(), order_direction, OrderRole::EXIT,
                 position->get_quantity(), OrderType::STOP, trigger_price, std::nullopt,
                 std::nullopt, position);
}

Order Order::make_take_profit(const std::shared_ptr<Position> &position, double trigger_price) {
    Direction order_direction = opposite(position->get_direction());

    if ((position->get_direction() == Direction::SHORT &&
         trigger_price >= position->get_entry_price()) ||
        (position->get_direction() == Direction::LONG &&
         trigger_price <= position->get_entry_price())) {
        throw std::logic_error("Invalid entry price for take profit order");
    }

    return Order(position->get_instrument(), order_direction, OrderRole::EXIT,
                 position->get_quantity(), OrderType::LIMIT, trigger_price, std::nullopt,
                 std::nullopt, position);
}

bool Order::assign_position(const std::shared_ptr<Position> &position) {
    if (role_ != OrderRole::ENTRY || !position_.expired()) {
        return false;
    }

    position_ = position;
    return true;
}

bool Order::execute() {
    if (status_ != OrderStatus::PENDING) {
        return false;
    }

    status_ = OrderStatus::EXECUTED;
    return true;
}

bool Order::cancel() {
    if (status_ != OrderStatus::PENDING) {
        return false;
    }

    status_ = OrderStatus::CANCELED;
    return true;
}

bool Order::convert_to_market() {
    if (type_ != OrderType::STOP) {
        return false;
    }

    type_ = OrderType::MARKET;
    return true;
}

uint32_t Order::get_id() const {
    return id_;
}

OrderStatus Order::get_status() const {
    return status_;
}

OrderRole Order::get_role() const {
    return role_;
}

bool Order::is_exit_order() const {
    return role_ == OrderRole::EXIT;
}

bool Order::is_entry_order() const {
    return role_ == OrderRole::ENTRY;
}

const Instrument &Order::get_instrument() const {
    return instrument_;
}

Direction Order::get_direction() const {
    return direction_;
}

double Order::get_volume() const {
    return volume_;
}

OrderType Order::get_type() const {
    return type_;
}

std::optional<double> Order::get_trigger_price() const {
    return trigger_price_;
}

const std::weak_ptr<Position> &Order::get_position() const {
    return position_;
}

std::optional<double> Order::get_stop_loss_price() const {
    return stop_loss_price_;
}

std::optional<double> Order::get_take_profit_price() const {
    return take_profit_price_;
}

Order::Order(const Instrument &instrument, Direction direction, OrderRole role, double volume,
             OrderType type, std::optional<double> trigger_price,
             std::optional<double> stop_loss_price, std::optional<double> take_profit_price,
             const std::shared_ptr<Position> &position)
    : id_{++last_order_id}, role_{role}, instrument_{instrument}, direction_{direction},
      volume_{volume}, type_{type}, trigger_price_{trigger_price}, position_{position},
      stop_loss_price_{stop_loss_price}, take_profit_price_{take_profit_price} {}
