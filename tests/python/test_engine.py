from pathlib import Path

import _backtester


def test_long_trade_makes_expected_profit(tmp_path):
    data_file = tmp_path / "EURUSD.csv"

    instrument = _backtester.Instrument(
        "EURUSD",
        100000,
        0.0001,
        100000,
    )

    open_price = 1.285
    high_price = 1.288
    low_price = 1.284
    close_price = 1.287
    delta = 0.001

    rows = []

    for timestamp in range(40):
        rows.append(
            f"{timestamp}, {open_price}, {high_price}, "
            f"{low_price}, {close_price}\n"
        )

        open_price += delta
        high_price += delta
        low_price += delta
        close_price += delta

    data_file.write_text("".join(rows))

    class AlwaysLongStrategy(_backtester.Strategy):
        def on_market(self, event):
            data = event.data

            self.send_signal(
                _backtester.Signal.make_market(
                    instrument,
                    _backtester.Direction.LONG,
                    data.low() + 0.002,
                    data.close() + 0.008,
                )
            )

    data_feed = _backtester.DataFeed(
        str(data_file),
        instrument,
    )

    portfolio_config = _backtester.PortfolioConfig(
        account_id=1,
        account_balance=10000,
    )

    risk_manager_config = _backtester.RiskManagerConfig(
        risk_per_trade=0.01,
        total_account_risk=0.1,
        sl_tp_ratio=0.4,
    )

    execution_config = _backtester.ExecutionConfig(
        margin_rate=0.05,
        maintenance_margin_rate=0.05,
    )

    strategy = AlwaysLongStrategy()

    engine = _backtester.Engine(
        strategy,
        data_feed,
        portfolio_config,
        risk_manager_config,
        execution_config,
    )

    metrics = engine.run()

    assert metrics.account_balance >= portfolio_config.account_balance + 1
    assert metrics.realized_pnl >= 1
    assert metrics.maximum_drawdown == portfolio_config.account_balance
    assert metrics.win_rate >= 0.8

    print()
    print(metrics.account_balance)
    print(metrics.realized_pnl)
    print(metrics.maximum_drawdown)
    print(metrics.win_rate)
