#pragma once

#include "backtester/events/MarketEvent.hpp"
#include "backtester/models/Instrument.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/utils/CSVFileLoader.hpp"
#include <string>
#include <vector>

class DataFeed {
  public:
    DataFeed(const std::string &filename, const Instrument &instrument) {
        data_ = CSVFileLoader::load_file(filename, instrument);
        current_ = data_.begin();
    }

    bool has_next() const {
        return current_ != data_.end();
    }

    MarketEvent get_next_market_event() {
        return *current_++;
    }

    MarketData get_current_market_data() {
        return *current_;
    }

  private:
    std::vector<MarketData>::iterator current_;
    std::vector<MarketData> data_;
};
