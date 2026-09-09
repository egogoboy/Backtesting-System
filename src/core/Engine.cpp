#include "backtester/core/Engine.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/models/Metrics.hpp"
#include <memory>

Metrics Engine::run() {
    while (data_feed_.get().has_next()) {
        event_queue_.push(std::make_shared<MarketEvent>(data_feed_.get().get_next_market_event()));
        while (!event_queue_.empty()) {
            std::shared_ptr<Event> event = event_queue_.front();
            event_queue_.pop();

            switch (event->get_type()) {
            case EventType::MARKET:
                execution_handler_.on_market_event(std::static_pointer_cast<MarketEvent>(event));
                risk_manager_.on_market_event(std::static_pointer_cast<MarketEvent>(event));
                metric_calculator_.on_market_event(std::static_pointer_cast<MarketEvent>(event));
                strategy_->on_market(*std::static_pointer_cast<MarketEvent>(event));
                break;

            case EventType::SIGNAL:
                risk_manager_.on_signal_event(std::static_pointer_cast<SignalEvent>(event));
                metric_calculator_.on_signal_event(std::static_pointer_cast<SignalEvent>(event));
                break;

            case EventType::ORDER:
                execution_handler_.on_order_event(std::static_pointer_cast<OrderEvent>(event));
                metric_calculator_.on_order_event(std::static_pointer_cast<OrderEvent>(event));
                break;

            case EventType::FILL:
                portfolio_.on_fill_event(std::static_pointer_cast<FillEvent>(event));
                metric_calculator_.on_fill_event(std::static_pointer_cast<FillEvent>(event));
                break;
            }
        }
    }

    return metric_calculator_.calculate_metrics();
}
