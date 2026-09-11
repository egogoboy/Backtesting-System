#include "backtester/core/Engine.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Signal.hpp"
#include "backtester/strategy/Strategy.hpp"
#include "test_utils.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <ostream>

void print_bar(std::ostream &out, const MarketData &bar) {
    out << bar.get_timestamp() << ", " << bar.get_open() << ", " << bar.get_high() << ", "
        << bar.get_low() << ", " << bar.get_close() << "\n";
}

PortfolioConfig portfolio_config(1, 10000);
RiskManagerConfig risk_manager_config(0.01, 0.1, 0.4);
ExecutionConfig execution_config(0.05, 0.05);

TEST(Backtest, LongTradeMakesExpectedProfit) {
    std::string temp_file = "temporary_csv_file.csv";

    std::ofstream fout(temp_file);

    MarketData bar{GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0};

    double delta = 0.001;

    for (int i = 0; i < 40; ++i) {
        print_bar(fout, bar);

        MarketData new_bar{GLOBAL_EURUSD_INSTRUMENT,
                           {bar.get_open() + delta, bar.get_high() + delta, bar.get_low() + delta,
                            bar.get_close() + delta},
                           bar.get_timestamp() + 1};

        bar = new_bar;
    }

    fout.close();

    class AlwaysLongStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            auto data = event.get_data();
            send_signal(Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG,
                                            data.get_low() + 0.002, data.get_close() + 0.008));
        }
    };

    DataFeed data_feed("temporary_csv_file.csv", GLOBAL_EURUSD_INSTRUMENT);

    auto strategy = std::make_shared<AlwaysLongStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_GE(metrics.account_balance, portfolio_config.account_balance + 1);
    EXPECT_GE(metrics.realized_pnl, 1);
    EXPECT_EQ(metrics.maximum_drawdown, portfolio_config.account_balance);
    EXPECT_GE(metrics.win_rate, 0.8);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, LongTradeMakesExpectedLoss) {
    std::string temp_file = "temporary_csv_file.csv";

    std::ofstream fout(temp_file);

    MarketData bar{GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0};

    double delta = -0.001;

    for (int i = 0; i < 40; ++i) {
        print_bar(fout, bar);

        MarketData new_bar{GLOBAL_EURUSD_INSTRUMENT,
                           {bar.get_open() + delta, bar.get_high() + delta, bar.get_low() + delta,
                            bar.get_close() + delta},
                           bar.get_timestamp() + 1};

        bar = new_bar;
    }

    fout.close();

    class AlwaysLongStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            auto data = event.get_data();
            send_signal(Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG,
                                            data.get_low(), data.get_close() + 0.005));
        }
    };

    DataFeed data_feed("temporary_csv_file.csv", GLOBAL_EURUSD_INSTRUMENT);

    auto strategy = std::make_shared<AlwaysLongStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LE(metrics.maximum_drawdown, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.win_rate, 0.1);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, ShortTradeMakesExpectedProfit) {
    std::string temp_file = "temporary_csv_file.csv";

    std::ofstream fout(temp_file);

    MarketData bar{GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0};

    double delta = -0.001;

    for (int i = 0; i < 40; ++i) {
        print_bar(fout, bar);

        MarketData new_bar{GLOBAL_EURUSD_INSTRUMENT,
                           {bar.get_open() + delta, bar.get_high() + delta, bar.get_low() + delta,
                            bar.get_close() + delta},
                           bar.get_timestamp() + 1};

        bar = new_bar;
    }

    fout.close();

    class AlwaysShortStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            auto data = event.get_data();
            send_signal(Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT,
                                            data.get_close(), data.get_low() - 0.004));
        }
    };

    DataFeed data_feed("temporary_csv_file.csv", GLOBAL_EURUSD_INSTRUMENT);

    auto strategy = std::make_shared<AlwaysShortStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_GE(metrics.account_balance, portfolio_config.account_balance + 1);
    EXPECT_GE(metrics.realized_pnl, 1);
    EXPECT_EQ(metrics.maximum_drawdown, portfolio_config.account_balance);
    EXPECT_GE(metrics.win_rate, 0.8);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, ShortTradeMakesExpectedLoss) {
    std::string temp_file = "temporary_csv_file.csv";

    std::ofstream fout(temp_file);

    MarketData bar{GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0};

    double delta = 0.001;

    for (int i = 0; i < 40; ++i) {
        print_bar(fout, bar);

        MarketData new_bar{GLOBAL_EURUSD_INSTRUMENT,
                           {bar.get_open() + delta, bar.get_high() + delta, bar.get_low() + delta,
                            bar.get_close() + delta},
                           bar.get_timestamp() + 1};

        bar = new_bar;
    }

    fout.close();

    class AlwaysLongStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            auto data = event.get_data();
            send_signal(Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT,
                                            data.get_high() + 0.004, data.get_close() - 0.01));
        }
    };

    DataFeed data_feed("temporary_csv_file.csv", GLOBAL_EURUSD_INSTRUMENT);

    auto strategy = std::make_shared<AlwaysLongStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LE(metrics.maximum_drawdown, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.win_rate, 0.1);

    std::filesystem::remove(temp_file);
}

void write_bars(const std::string &file_name, const std::vector<MarketData> &bars) {
    std::ofstream fout(file_name);

    for (const auto &bar : bars) {
        print_bar(fout, bar);
    }
}

TEST(Backtest, LongLimitOrderMakesExpectedProfit) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{{GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

                                 {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

                                 {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 2},

                                 // Price goes down through the limit entry.
                                 {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.288, 1.2835, 1.285}, 3},

                                 // Price moves up and reaches TP.
                                 {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.291, 1.284, 1.290}, 4}};

    write_bars(temp_file, bars);

    class LongLimitStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG,
                                           1.285, // entry
                                           1.283, // stop loss
                                           1.290  // take profit
                                           ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<LongLimitStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_GE(metrics.account_balance, portfolio_config.account_balance + 1);
    EXPECT_GE(metrics.realized_pnl, 1);
    EXPECT_GT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, LongLimitOrderMakesExpectedLoss) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{{GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

                                 {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

                                 {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 2},

                                 // Limit entry is triggered.
                                 {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.288, 1.2835, 1.285}, 3},

                                 // Price falls through SL.
                                 {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.286, 1.282, 1.283}, 4}

    };

    write_bars(temp_file, bars);

    class LongLimitStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG,
                                           1.285, // entry
                                           1.283, // stop loss
                                           1.290  // take profit
                                           ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<LongLimitStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}
TEST(Backtest, ShortLimitOrderMakesExpectedProfit) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{
        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

        // Price rises through the limit entry.
        {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.289, 1.286, 1.288}, 2},

        // Price falls and reaches TP.
        {GLOBAL_EURUSD_INSTRUMENT, {1.288, 1.289, 1.279, 1.283}, 3},
    };

    write_bars(temp_file, bars);

    class ShortLimitStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT,
                                           1.287, // entry
                                           1.289, // stop loss
                                           1.280  // take profit
                                           ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<ShortLimitStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_GE(metrics.account_balance, portfolio_config.account_balance + 1);
    EXPECT_GE(metrics.realized_pnl, 1);
    EXPECT_GT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, ShortLimitOrderMakesExpectedLoss) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{
        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

        // Limit entry is triggered.
        {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.289, 1.286, 1.288}, 2},

        // Price rises through SL.
        {GLOBAL_EURUSD_INSTRUMENT, {1.288, 1.291, 1.287, 1.290}, 3},
    };

    write_bars(temp_file, bars);

    class ShortLimitStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_limit(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT,
                                           1.287, // entry
                                           1.290, // stop loss
                                           1.278  // take profit
                                           ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<ShortLimitStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, LongStopOrderMakesExpectedProfit) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{
        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

        // Price breaks upward through stop entry.
        {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.291, 1.286, 1.290}, 2},

        // Price continues upward and reaches TP.
        {GLOBAL_EURUSD_INSTRUMENT, {1.290, 1.296, 1.289, 1.295}, 3},
    };

    write_bars(temp_file, bars);

    class LongStopStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG,
                                          1.290, // entry
                                          1.289, // stop loss
                                          1.295  // take profit
                                          ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<LongStopStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_GE(metrics.account_balance, portfolio_config.account_balance + 1);
    EXPECT_GE(metrics.realized_pnl, 1);
    EXPECT_GT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, LongStopOrderMakesExpectedLoss) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{
        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

        // Stop entry is triggered.
        {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.291, 1.286, 1.290}, 2},

        // Price reverses and reaches SL.
        {GLOBAL_EURUSD_INSTRUMENT, {1.290, 1.291, 1.286, 1.287}, 3},
    };

    write_bars(temp_file, bars);

    class LongStopStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG,
                                          1.290, // entry
                                          1.287, // stop loss
                                          1.299  // take profit
                                          ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<LongStopStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, ShortStopOrderMakesExpectedProfit) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{
        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

        // Price breaks downward through stop entry.
        {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.288, 1.283, 1.284}, 2},

        // Price continues downward and reaches TP.
        {GLOBAL_EURUSD_INSTRUMENT, {1.284, 1.285, 1.279, 1.280}, 3},
    };

    write_bars(temp_file, bars);

    class ShortStopStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT,
                                          1.284, // entry
                                          1.285, // stop loss
                                          1.280  // take profit
                                          ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<ShortStopStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_GE(metrics.account_balance, portfolio_config.account_balance + 1);
    EXPECT_GE(metrics.realized_pnl, 1);
    EXPECT_GT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}

TEST(Backtest, ShortStopOrderMakesExpectedLoss) {
    std::string temp_file = "temporary_csv_file.csv";

    std::vector<MarketData> bars{
        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 0},

        {GLOBAL_EURUSD_INSTRUMENT, {1.285, 1.288, 1.284, 1.287}, 1},

        // Stop entry is triggered.
        {GLOBAL_EURUSD_INSTRUMENT, {1.287, 1.288, 1.283, 1.284}, 2},

        // Price reverses upward and reaches SL.
        {GLOBAL_EURUSD_INSTRUMENT, {1.284, 1.289, 1.283, 1.288}, 3},
    };

    write_bars(temp_file, bars);

    class ShortStopStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_stop(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT,
                                          1.284, // entry
                                          1.287, // stop loss
                                          1.275  // take profit
                                          ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<ShortStopStrategy>();

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LT(metrics.win_rate, 0.5);

    std::filesystem::remove(temp_file);
}

TEST(BacktesterPipelineTest, LongPositionIsLiquidatedOnMarginCall) {

    class SingleLongSignalStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::LONG,
                                            0.75, // stop loss
                                            8.30  // take profit
                                            ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    std::vector<MarketData> bars;

    bars.emplace_back(MarketData(GLOBAL_EURUSD_INSTRUMENT,
                                 {4.00,  // open
                                  4.001, // high
                                  3.999, // low
                                  4.00}, // close
                                 1));

    // Gradually move against the long position.
    for (int i = 1; i <= 10; ++i) {
        double price = 2.00 - i * 0.1;

        bars.emplace_back(
            MarketData(GLOBAL_EURUSD_INSTRUMENT, {price, price + 0.001, price - 0.001, price}, 1));
    }

    std::string temp_file = "temporary_csv_file.csv";

    write_bars(temp_file, bars);

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<SingleLongSignalStrategy>();

    RiskManagerConfig risk_manager_config(0.9, 1, 1);
    ExecutionConfig execution_config(0.99, 0.98);

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LT(metrics.win_rate, 0.5);
    EXPECT_EQ(metrics.amount_of_executed_orders, 1);
    EXPECT_LT(metrics.account_balance, 10000);
    EXPECT_LT(metrics.realized_pnl, 0);

    std::filesystem::remove(temp_file);
}

TEST(BacktesterPipelineTest, ShortPositionIsLiquidatedOnMarginCall) {

    class SingleShortSignalStrategy : public Strategy {
      public:
        void on_market(const MarketEvent &event) override {
            if (sent_) {
                return;
            }

            send_signal(Signal::make_market(GLOBAL_EURUSD_INSTRUMENT, Direction::SHORT,
                                            8.30, // stop loss
                                            0.75  // take profit
                                            ));

            sent_ = true;
        }

      private:
        bool sent_ = false;
    };

    std::vector<MarketData> bars;

    bars.emplace_back(MarketData(GLOBAL_EURUSD_INSTRUMENT,
                                 {4.00,  // open
                                  4.001, // high
                                  3.999, // low
                                  4.00}, // close
                                 1));

    // Gradually move against the long position.
    for (int i = 1; i <= 10; ++i) {
        double price = 7.00 + i * 0.1;

        bars.emplace_back(
            MarketData(GLOBAL_EURUSD_INSTRUMENT, {price, price + 0.001, price - 0.001, price}, 1));
    }

    std::string temp_file = "temporary_csv_file.csv";

    write_bars(temp_file, bars);

    DataFeed data_feed(temp_file, GLOBAL_EURUSD_INSTRUMENT);
    auto strategy = std::make_shared<SingleShortSignalStrategy>();

    RiskManagerConfig risk_manager_config(0.9, 1, 1);
    ExecutionConfig execution_config(0.99, 0.98);

    Engine engine(strategy, data_feed, portfolio_config, risk_manager_config, execution_config);

    auto metrics = engine.run();

    EXPECT_LE(metrics.account_balance, portfolio_config.account_balance - 1);
    EXPECT_LE(metrics.realized_pnl, -1);
    EXPECT_LT(metrics.win_rate, 0.5);
    EXPECT_EQ(metrics.amount_of_executed_orders, 1);
    EXPECT_LT(metrics.account_balance, 10000);
    EXPECT_LT(metrics.realized_pnl, 0);

    std::filesystem::remove(temp_file);
}
