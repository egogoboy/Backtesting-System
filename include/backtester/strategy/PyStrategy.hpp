#pragma once

#include "backtester/strategy/Strategy.hpp"
#include <pybind11/pybind11.h>

class PyStrategy : public Strategy {
  public:
    using Strategy::Strategy;

    void on_market(const MarketEvent &event) override {
        PYBIND11_OVERLOAD_PURE(void, Strategy, on_market, event);
    }

    using Strategy::send_signal;
};
