#pragma once

#include "backtester/core/EventQueue.hpp"
#include "backtester/events/MarketEvent.hpp"
#include "backtester/events/SignalEvent.hpp"
#include "backtester/models/Signal.hpp"
#include <memory>

class Engine;

class Strategy {
  public:
    virtual void on_market(const MarketEvent &event) = 0;

    Strategy() = default;
    Strategy(const Strategy &other) = default;
    Strategy(Strategy &&other) = default;
    Strategy &operator=(const Strategy &other) = default;
    Strategy &operator=(Strategy &&other) = default;
    virtual ~Strategy() = default;

  protected:
    void send_signal(const Signal &signal) {
        if (event_queue_ == nullptr) {
            return;
        }
        event_queue_->push(std::make_shared<SignalEvent>(signal));
    }

  private:
    friend Engine;

    void set_event_queue(EventQueue &queue) {
        event_queue_ = &queue;
    }

    EventQueue *event_queue_ = nullptr;
};
