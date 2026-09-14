import _backtester


def test_execution_config():
    config = _backtester.ExecutionConfig(
        margin_rate = 0.05,
        maintenance_margin_rate = 0.5
    )

    assert config.margin_rate == 0.05
    assert config.maintenance_margin_rate == 0.5


def test_portfolio_config():
    config = _backtester.PortfolioConfig(
        account_id = 123,
        account_balance = 10000.0
    )

    assert config.account_id == 123
    assert config.account_balance == 10000.0


def test_risk_manager_config():
    config = _backtester.RiskManagerConfig(
        risk_per_trade = 0.01,
        total_account_risk = 0.05,
        sl_tp_ratio = 2.0
    )

    assert config.risk_per_trade == 0.01
    assert config.total_account_risk == 0.05
    assert config.sl_tp_ratio == 2.0
