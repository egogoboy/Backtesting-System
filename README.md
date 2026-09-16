# Backtesting System

Event-driven algorithmic trading backtesting engine written in modern C++20 with modular architecture and Python interface.

![CI](https://github.com/egogoboy/Backtesting-System/actions/workflows/ci.yml/badge.svg)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Python](https://img.shields.io/badge/Python-3.x-blue)
![pybind11](https://img.shields.io/badge/pybind11-blue)
![CMake](https://img.shields.io/badge/CMake-3.24%2B-blue)
![GoogleTest](https://img.shields.io/badge/GoogleTest-blue)
![PyTest](https://img.shields.io/badge/PyTest-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Overview

## Architecture

<p align="center">
    <img src="https://github.com/egogoboy/Backtesting-System/blob/main/docs/images/Event%20Flow%20Diagram.svg" width="80%" alt="Event Flow Diagram">
</p>

## Features



## Example

A minimal example of running a backtest with a custom strategy:

```py
import backtester

# Configure trading instrument
instrument = backtester.Instrument('EURUSD', 100000, 0.0001, 100000)

class AlwaysLongStrategy(backtester.Strategy):
    def on_market(self, event):
        data = event.data
        # Generate a long market signal with stop-loss and take-profit.
        self.send_signal(
            backtester.Signal.make_market(
                instrument,
                backtester.Direction.LONG,
                data.low() + 0.002,    # Stop-loss
                data.close() + 0.008,  # Take-profit
            )
        )

# Load historical market data.
data_feed = backtester.DataFeed(
    'data/EURUSD.csv',
    instrument,
)

# Configure the portfolio and account.
portfolio_config = backtester.PortfolioConfig(
    account_id=1,
    account_balance=10000,
)

# Configure position and account-level risk limits.
risk_manager_config = backtester.RiskManagerConfig(
    risk_per_trade=0.01,
    total_account_risk=0.1,
    sl_tp_ratio=0.4,
)

# Configure margin requirements.
execution_config = backtester.ExecutionConfig(
    margin_rate=0.05,
    maintenance_margin_rate=0.05,
)

# Create the strategy and run the backtest.
strategy = AlwaysLongStrategy()

engine = backtester.Engine(
    strategy,
    data_feed,
    portfolio_config,
    risk_manager_config,
    execution_config,
)

result = engine.run()
print("Backtesting Results")
print("-------------------")
print(f"Account balance:   {result.account_balance:.2f}")
print(f"Realized PnL:      {result.realized_pnl:.2f}")
print(f"Maximum drawdown:  {result.maximum_drawdown:.2%}")
print(f"Win rate:          {result.win_rate:.2%}")
```

**Sample output:**

```
Backtesting Results
-------------------
Account balance:   10284.50
Realized PnL:      284.50
Maximum drawdown:  3.72%
Win rate:          61.54%
```

## Installation

### Requirements



### Build

### Run tests



## Project Structure

```sh
.
├── apps/             # Application entry point (executable targets)
├── CMakeLists.txt    # Main CMake build configuration file
├── docs/             # Project documentation and guides
├── include/          # Public header files (.hpp)
│   └── backtester/   # Library namespace directory
├── python/           # Python bindings and wrapper source code
├── README.md         # Project overview and setup instructions
├── src/              # C++ implementation files (.cpp)
└── tests/            # Unit and integration tests
    ├── cpp/          # C++ test suites (GTest)
    └── python/       # Python test suites (pytest)
```

## Limitations



## Roadmap

- [ ] Python package
- [ ] Basic OHLC csv file support
- [ ] Isolated margin
- [ ] High/Low check for Stop Out
- [ ] Performance metrics API
- [ ] More historical data formats
- [ ] Strategy optimization
- [ ] Walk-forward analysis

## Project Status

## Author

**Egor Sudakov**

[GitHub](https://github.com/egogoboy) ·
[LinkedIn](https://linkedin.com/in/erian-sunavell) ·
[Website](https://personal-website-theta-ruddy-16.vercel.app)

[Send me an email](mailto:erian.sunavell@gmail.com)

## License

This project is licensed under the MIT License.
