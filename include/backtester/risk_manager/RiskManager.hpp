#pragma once

#include "backtester/config/RiskManagerConfig.hpp"
#include "backtester/core/EventQueue.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/events/SignalEvent.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Order.hpp"
#include "backtester/models/Signal.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include <functional>
#include <memory>

class RiskManager {
  public:
    RiskManager(const RiskManagerConfig &config, EventQueue &event_queue,
                const Portfolio &portfolio, const MarketData &init_market_data);

    void on_signal_event(const std::shared_ptr<SignalEvent> &event) const;

    void on_market_event(const std::shared_ptr<MarketEvent> &event);

  private:
    void place_order(const std::shared_ptr<Order> &order) const;

    static std::shared_ptr<Order> create_order(const Signal &signal, double position_size);

    bool check_stop_loss_take_profit(const Signal &signal) const;

    static bool check_signal_structure(const Signal &signal);

    bool check_stop_loss_take_profit_ratio(const Signal &signal) const;

    bool check_total_accout_risk(double safe_capital) const;

    double calculate_safe_capital() const;

    double calculate_position_size(const Signal &signal, double safe_capital) const;

    double get_signal_entry_price(const Signal &signal) const;

    std::reference_wrapper<EventQueue> event_queue_;
    std::reference_wrapper<const Portfolio> portfolio_;
    std::reference_wrapper<const MarketData> last_market_data_;

    RiskManagerConfig config_;
};
