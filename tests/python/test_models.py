import _backtester


def test_ohlc():
    ohlc = _backtester.OHLC(
        1, 2, 3, 4
    )

    assert ohlc.open == 1
    assert ohlc.high == 2
    assert ohlc.low == 3
    assert ohlc.close == 4


def test_instrument():
    instrument = _backtester.Instrument('EURUSD', 0.1, 0.2, 0.3)

    assert instrument.symbol == 'EURUSD'
    assert instrument.contract_size == 0.1
    assert instrument.pip_size == 0.2
    assert instrument.lot_size == 0.3


def test_market_signal():
    instrument = _backtester.Instrument('EURUSD', 0.1, 0.2, 0.3)

    signal = _backtester.Signal.make_market(instrument, _backtester.Direction.LONG, 0.1, 0.1)

    assert signal.type == _backtester.SignalType.MARKET
    assert signal.instrument == instrument
    assert signal.direction == _backtester.Direction.LONG
    assert signal.stop_loss_price == 0.1
    assert signal.take_profit_price == 0.1


def test_limit_signal():
    instrument = _backtester.Instrument('EURUSD', 0.1, 0.2, 0.3)

    signal = _backtester.Signal.make_limit(instrument, _backtester.Direction.LONG, 0.1, 0.1, 0.1)

    assert signal.type == _backtester.SignalType.LIMIT
    assert signal.instrument == instrument
    assert signal.direction == _backtester.Direction.LONG
    assert signal.entry_price == 0.1
    assert signal.stop_loss_price == 0.1
    assert signal.take_profit_price == 0.1


def test_stop_signal():
    instrument = _backtester.Instrument('EURUSD', 0.1, 0.2, 0.3)

    signal = _backtester.Signal.make_stop(instrument, _backtester.Direction.LONG, 0.1, 0.1, 0.1)

    assert signal.type == _backtester.SignalType.STOP
    assert signal.instrument == instrument
    assert signal.direction == _backtester.Direction.LONG
    assert signal.entry_price == 0.1
    assert signal.stop_loss_price == 0.1
    assert signal.take_profit_price == 0.1


def test_metrics():
    metrics = _backtester.Metrics(
        account_balance = 1,
        realized_pnl = 1,
        unrealized_pnl = 1,
        win_rate = 1,
        maximum_drawdown = 1,
        profit_factor = 1,
        expectancy_money = 1,
        expectancy_r = 1,
        amount_of_orders = 1,
        amount_of_signals = 1,
        amount_of_executed_orders = 1
    )

    assert metrics.account_balance == 1
    assert metrics.realized_pnl == 1
    assert metrics.unrealized_pnl == 1
    assert metrics.win_rate == 1
    assert metrics.maximum_drawdown == 1
    assert metrics.profit_factor == 1
    assert metrics.expectancy_money == 1
    assert metrics.expectancy_r == 1
    assert metrics.amount_of_orders == 1
    assert metrics.amount_of_signals == 1
    assert metrics.amount_of_executed_orders == 1
