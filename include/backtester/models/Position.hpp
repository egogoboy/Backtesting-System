#pragma once

#include "backtester/enums/Direction.hpp"
#include "backtester/enums/PositionStatus.hpp"
#include "backtester/models/Instrument.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

class Order;

class Position {
  public:
    Position(const Instrument &instrument, double quantity, Direction direction, double entry_price)
        : id_{++last_position_id}, instrument_{instrument}, quantity_{quantity},
          direction_{direction}, entry_price_{entry_price} {}

    bool close_position(double exit_price) {
        if (status_ != PositionStatus::OPEN) {
            return false;
        }

        status_ = PositionStatus::CLOSE;

        exit_price_ = exit_price;
        return true;
    }

    uint32_t get_id() const {
        return id_;
    }

    const Instrument &get_instrument() const {
        return instrument_;
    }

    double get_quantity() const {
        return quantity_;
    }

    Direction get_direction() const {
        return direction_;
    }

    double get_entry_price() const {
        return entry_price_;
    }

    std::optional<double> get_exit_price() const {
        return exit_price_;
    }

    bool set_stop_loss_order(const std::shared_ptr<Order> &order) {
        if (!stop_loss_order_.lock()) {
            return false;
        }

        stop_loss_order_ = order;
        return true;
    }

    std::weak_ptr<Order> get_stop_loss_order() const {
        return stop_loss_order_;
    }

    bool set_take_profit_order(const std::shared_ptr<Order> &order) {
        if (!take_profit_order_.lock()) {
            return false;
        }

        take_profit_order_ = order;
        return true;
    }

    std::weak_ptr<Order> get_take_profit_order() const {
        return take_profit_order_;
    }

    PositionStatus get_status() const {
        return status_;
    }

  private:
    uint32_t id_;
    std::reference_wrapper<const Instrument> instrument_;
    double quantity_;
    Direction direction_;
    double entry_price_;
    std::optional<double> exit_price_;
    std::weak_ptr<Order> stop_loss_order_;
    std::weak_ptr<Order> take_profit_order_;
    PositionStatus status_ = PositionStatus::OPEN;

    static inline uint32_t last_position_id = 0;
};
