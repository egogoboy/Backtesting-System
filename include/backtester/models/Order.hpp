#pragma once

#include "backtester/enums/Direction.hpp"
#include "backtester/enums/OrderRole.hpp"
#include "backtester/enums/OrderStatus.hpp"
#include "backtester/enums/OrderType.hpp"
#include "backtester/models/Instrument.hpp"

#include <cstdint>
#include <memory>

class Position;

class Order {
  public:
    static Order make_market(const Instrument &instrument, Direction direction, double volume,
                             double stop_loss_price, double take_profit_price);

    static Order make_limit(const Instrument &instrument, Direction direction, double volume,
                            double trigger_price, double stop_loss_price, double take_profit_price);

    static Order make_stop(const Instrument &instrument, Direction direction, double volume,
                           double trigger_price, double stop_loss_price, double take_profit_price);

    static Order make_stop_loss(const std::shared_ptr<Position> &position, double trigger_price);

    static Order make_take_profit(const std::shared_ptr<Position> &position, double trigger_price);

    bool execute();

    bool cancel();

    bool convert_to_market();

    uint32_t get_id() const;

    OrderStatus get_status() const;

    OrderRole get_role() const;

    bool is_exit_order() const;

    bool is_entry_order() const;

    const Instrument &get_instrument() const;

    Direction get_direction() const;

    double get_volume() const;

    OrderType get_type() const;

    std::optional<double> get_trigger_price() const;

    const std::weak_ptr<Position> &get_position() const;

    std::optional<double> get_stop_loss_price() const;

    std::optional<double> get_take_profit_price() const;

  private:
    Order(const Instrument &instrument, Direction direction, OrderRole role, double volume,
          OrderType type, std::optional<double> trigger_price,
          std::optional<double> stop_loss_price, std::optional<double> take_profit_price,
          const std::shared_ptr<Position> &position);

    uint32_t id_;
    OrderStatus status_ = OrderStatus::PENDING;
    OrderRole role_ = OrderRole::ENTRY;
    std::reference_wrapper<const Instrument> instrument_;
    Direction direction_;
    double volume_;
    OrderType type_;
    std::optional<double> trigger_price_;
    std::weak_ptr<Position> position_;

    std::optional<double> stop_loss_price_;
    std::optional<double> take_profit_price_;

    static inline uint32_t last_order_id = 0;
};
