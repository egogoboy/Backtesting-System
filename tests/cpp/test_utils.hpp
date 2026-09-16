#pragma once

#include "backtester/models/MarketData.hpp"

const inline Instrument GLOBAL_EURUSD_INSTRUMENT("EURUSD", 100000, 0.0001, 100000);

inline bool compare_market_data(const MarketData &lhs, const MarketData &rhs) {
    return lhs.get_open() == rhs.get_open() && lhs.get_high() == rhs.get_high() &&
           lhs.get_low() == rhs.get_low() && lhs.get_close() == rhs.get_close() &&
           lhs.get_instrument().get_contract_size() == rhs.get_instrument().get_contract_size() &&
           lhs.get_instrument().get_lot_size() == rhs.get_instrument().get_lot_size() &&
           lhs.get_instrument().get_pip_size() == rhs.get_instrument().get_pip_size() &&
           lhs.get_instrument().get_symbol() == rhs.get_instrument().get_symbol();
}
