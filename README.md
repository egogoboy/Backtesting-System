# Backtesting System

Event-driven algorithmic trading backtesting engine written in modern C++20 with a Python interface.

![CI](https://github.com/egogoboy/Backtesting-System/actions/workflows/ci.yml/badge.svg)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![Python](https://img.shields.io/badge/Python-3.x-blue)
![pybind11](https://img.shields.io/badge/pybind11-blue)
![CMake](https://img.shields.io/badge/CMake-3.24%2B-blue)
![GoogleTest](https://img.shields.io/badge/GoogleTest-blue)
![PyTest](https://img.shields.io/badge/PyTest-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Overview

Backtesting System is an event-driven framework for evaluating algorithmic trading strategies on historical market data. The project was developed to connect software engineering with the practical study of financial markets and to explore how trading strategies can be systematically tested through software.

The system is implemented in **C++20** and provides a **Python interface** for defining strategies and running backtests. Its architecture separates strategy logic from risk management, order execution, portfolio management, and performance analysis, with market data, signals, orders, and fills represented as events processed sequentially through the trading pipeline.

The project also serves as the foundation for further research into **strategy optimization and robustness**. Future development will focus on methods for evaluating whether backtest results generalize beyond a particular historical dataset, including approaches to overfitting, parameter stability, and walk-forward analysis.

## Architecture

<p align="center">
    <img src="https://github.com/egogoboy/Backtesting-System/blob/main/docs/images/Event%20Flow%20Diagram.svg" width="80%" alt="Event Flow Diagram">
</p>

The system follows an **event-driven architecture** in which the Engine coordinates the main components and manages the flow of events. Historical market data is converted into MarketEvent objects and passed to the strategy, which generates SignalEvent objects. Signals are evaluated by the Risk Manager and, when accepted, converted into OrderEvent objects for execution.

The Execution Handler processes orders and produces FillEvent objects, which update the Portfolio and are fed back into the event pipeline. The Metrics Calculator observes the trading process and collects performance data throughout the backtest. This separation keeps strategy logic independent from risk management, execution, and portfolio management.

## Features

### Market Data

The backtester currently works with **historical OHLC data** loaded from CSV files. Market data is processed sequentially and converted into market events, which are passed through the trading pipeline one step at a time. This event-driven approach keeps the order of operations explicit and prevents strategies from accessing future market data during a backtest.

### Strategies

Strategies are implemented through an **extensible strategy interface** that allows custom trading logic to be defined independently from the rest of the system. A strategy can analyse incoming market events and generate trading signals for both long and short positions, while the subsequent risk and execution stages remain handled by the backtesting engine.

### Orders

The system supports **market**, **limit**, and **stop orders**. Trading signals must specify stop-loss and take-profit levels, which are validated according to the direction of the position. This allows the same order and execution pipeline to be used for different types of trading scenarios without coupling strategy logic to order management.

### Execution

Order execution is simulated using execution parameters. Market, limit, and stop orders are processed according to their respective execution conditions, while stop-loss and take-profit orders are handled as part of the position lifecycle. _The execution model is intentionally simplified and is designed for historical strategy evaluation rather than detailed market microstructure simulation._

### Risk Management

**Risk management is applied before orders are submitted for execution.** The system supports **position-level** and **account-level** risk limits, configurable **leverage and margin** requirements, and **maintenance margin** monitoring. Account-level conditions such as margin calls and stop-out events are also simulated to model situations in which available capital becomes insufficient to maintain open positions.

### Portfolio

The portfolio tracks the state of the trading account throughout the backtest. It maintains **account balance and equity**, **realized and unrealized P&L**, **used and available margin**, **open positions**, and **risks associated with pending orders**. Portfolio state is updated as market events, orders, and fills move through the event-driven pipeline.

### Testing

The project is covered by **unit, integration, and end-to-end tests**. Tests verify individual components as well as complete trading scenarios involving strategies, risk management, order execution, and portfolio updates. AddressSanitizer and UndefinedBehaviorSanitizer are used during development to detect memory-safety and undefined-behavior issues, while **the full test suite is executed automatically in CI**.

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

- C++20 compiler
- CMake
- Python 3.x

### Build

```bash
git clone https://github.com/egogoboy/Backtesting-System
cd Backtesting-System
cmake -S . -B build
cmake --build build
```

### Run tests

```bash
ctest --test-dir build --output-on-failure
pytest --rootdir tests/python
```

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

The current implementation is intentionally focused on a simplified historical backtesting environment.

- System is primarily focused on modeling Forex market
- Only historical OHLC data is currently supported
- Market microstructure and order-book dynamics are not simulated
- The execution model does not account for queue position or detailed latency effects
- The current interface is focused on running backtests; strategy optimization and walk-forward analysis are planned extensions

## Roadmap

- [x] Python interface
- [x] Historical OHLC data from CSV
- [x] Margin and leverage
- [x] Performance metrics
- [ ] Basic OHLC CSV file support (with normal timestamp)
- [ ] Isolated margin
- [ ] High/Low check for Stop Out
- [ ] Performance metrics API
- [ ] More historical data formats
- [ ] Strategy optimization
- [ ] Walk-forward analysis

## Project Status

The core event-driven architecture and Python interface are fully implemented for the Minimum Viable Product (MVP) stage. Current development focuses on packaging the framework into a native Python library to facilitate seamless deployment for quantitative research.

## Author

**Egor Sudakov**

[GitHub](https://github.com/egogoboy) ·
[LinkedIn](https://linkedin.com/in/erian-sunavell) ·
[Website](https://personal-website-theta-ruddy-16.vercel.app)

[Send me an email](mailto:erian.sunavell@gmail.com)

## License

This project is licensed under the MIT License.
