#include "backtester/data_feed/DataFeed.hpp"
#include "test_utils.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

TEST(DataFeed, LoadFromFile) {
    std::string temp_file = "temporary_csv_file.csv";

    std::ofstream fout(temp_file);

    uint64_t time = 0;
    OHLC ohlc = {1, 2, 3, 4};
    std::vector<MarketData> raw_data;

    for (int i = 0; i < 5; ++i) {
        fout << time << ", " << ohlc.open << ", " << ohlc.high << ", " << ohlc.low << ", "
             << ohlc.close << '\n';

        raw_data.push_back(MarketData(GLOBAL_EURUSD_INSTRUMENT, ohlc, time));

        ++ohlc.open;
        ++ohlc.high;
        ++ohlc.low;
        ++ohlc.close;
        ++time;
    }

    fout.close();

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);

    std::filesystem::remove(temp_file);

    auto it = raw_data.begin();

    while (data_feed.has_next()) {
        MarketData md = data_feed.get_next_market_event().get_data();
        EXPECT_EQ(compare_market_data(*it++, md), true);
    }
}
