#pragma once

#include "backtester/enums/EventType.hpp"
#include "backtester/events/Event.hpp"
#include "backtester/models/Order.hpp"
#include <memory>

class OrderEvent : public Event {
  public:
    OrderEvent(const std::shared_ptr<Order> &order) : data_{order} {}

    EventType get_type() const override {
        return EventType::ORDER;
    }

    const std::shared_ptr<Order> &get_data() const {
        return data_;
    }

  private:
    std::shared_ptr<Order> data_;
};
