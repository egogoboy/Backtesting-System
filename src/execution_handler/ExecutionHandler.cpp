#include "backtester/execution_handler/ExecutionHandler.hpp"
#include "backtester/enums/Direction.hpp"
#include "backtester/enums/OrderType.hpp"

ExecutionHandler::ExecutionHandler(EventQueue &event_queue, Portfolio &portfolio,
                                   const MarketData &initial_market_data)
    : event_queue_{event_queue}, portfolio_{portfolio}, last_market_data_{initial_market_data} {}

void ExecutionHandler::on_market_event(const std::shared_ptr<MarketEvent> &event) {}

void ExecutionHandler::on_order_event(const std::shared_ptr<OrderEvent> &event) {
    Order order = std::move(event->get_data());

    double current_price = last_market_data_.get().get_close();
    Direction direction = order.get_direction();

    if (order.get_type() == OrderType::LIMIT) {
        double limit_price = order.get_trigger_price().value();

        if ((direction == Direction::LONG && limit_price > current_price) ||
            (direction == Direction::SHORT && limit_price < current_price)) {
            return;
        }
    } else if (order.get_type() == OrderType::STOP) {
        double stop_price = order.get_trigger_price().value();

        if ((direction == Direction::LONG && stop_price <= current_price) ||
            (direction == Direction::SHORT && stop_price >= current_price)) {
            return;
        }
    }

    orders_.emplace_back(std::move(order));
}

double ExecutionHandler::get_floating_risk() const {}

void ExecutionHandler::execute_order(Order &order) {}

void ExecutionHandler::fill_position(Order &order) {}

void ExecutionHandler::update_floating_risk(const Order &order) {}
