#pragma once

#include <string>
class Instrument {
  public:
    Instrument(const std::string &symbol, double contract_size, double pip_size, double lot_size)
        : symbol_{symbol}, contract_size_{contract_size}, pip_size_{pip_size}, lot_size_{lot_size} {
    }

    const std::string &get_symbol() const {
        return symbol_;
    }

    double get_contract_size() const {
        return contract_size_;
    }

    double get_pip_size() const {
        return pip_size_;
    }

    double get_lot_size() const {
        return lot_size_;
    }

  private:
    std::string symbol_;
    double contract_size_;
    double pip_size_;
    double lot_size_;
};
