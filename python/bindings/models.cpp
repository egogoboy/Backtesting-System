#include "backtester/enums/Direction.hpp"
#include "backtester/models/Instrument.hpp"
#include "backtester/models/MarketData.hpp"
#include "backtester/models/Metrics.hpp"
#include "backtester/models/Signal.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bind_models(py::module_ &m) {
    py::class_<OHLC>(m, "OHLC")
        .def(py::init<double, double, double, double>(), py::arg("open"), py::arg("high"),
             py::arg("low"), py::arg("close"))
        .def_readwrite("open", &OHLC::open)
        .def_readwrite("high", &OHLC::high)
        .def_readwrite("low", &OHLC::low)
        .def_readwrite("close", &OHLC::close);

    py::class_<Instrument>(m, "Instrument")
        .def(py::init<const std::string &, double, double, double>(), py::arg("symbol"),
             py::arg("contract_size"), py::arg("pip_size"), py::arg("lot_size"))
        .def_property_readonly("symbol", &Instrument::get_symbol)
        .def_property_readonly("contract_size", &Instrument::get_contract_size)
        .def_property_readonly("pip_size", &Instrument::get_pip_size)
        .def_property_readonly("lot_size", &Instrument::get_lot_size);

    py::class_<MarketData>(m, "MarketData")
        .def("instrument", &MarketData::get_instrument, py::return_value_policy::reference_internal)
        .def("open", &MarketData::get_open)
        .def("high", &MarketData::get_high)
        .def("low", &MarketData::get_low)
        .def("close", &MarketData::get_close)
        .def("timestamp", &MarketData::get_timestamp);

    py::class_<Signal>(m, "Signal")
        .def_static("make_market", &Signal::make_market, py::arg("instrument"),
                    py::arg("direction"), py::arg("stop_loss"), py::arg("take_profit"))
        .def_static("make_limit", &Signal::make_limit, py::arg("instrument"), py::arg("direction"),
                    py::arg("entry_price"), py::arg("stop_loss"), py::arg("take_profit"))
        .def_static("make_stop", &Signal::make_stop, py::arg("instrument"), py::arg("direction"),
                    py::arg("entry_price"), py::arg("stop_loss"), py::arg("take_profit"))
        .def_property_readonly("instrument", &Signal::get_instrument,
                               py::return_value_policy::reference_internal)
        .def_property_readonly("direction", &Signal::get_direction)
        .def_property_readonly("type", &Signal::get_type)
        .def_property_readonly("entry_price", &Signal::get_entry_price)
        .def_property_readonly("stop_loss_price", &Signal::get_stop_loss_price)
        .def_property_readonly("take_profit_price", &Signal::get_take_profit_price);

    py::class_<Metrics>(m, "Metrics")
        .def(py::init<double, double, double, double, double, double, double, double, int, int,
                      int>(),
             py::arg("account_balance"), py::arg("realized_pnl"), py::arg("unrealized_pnl"),
             py::arg("win_rate"), py::arg("maximum_drawdown"), py::arg("profit_factor"),
             py::arg("expectancy_money"), py::arg("expectancy_r"), py::arg("amount_of_orders"),
             py::arg("amount_of_signals"), py::arg("amount_of_executed_orders"))
        .def_readwrite("account_balance", &Metrics::account_balance)
        .def_readwrite("realized_pnl", &Metrics::realized_pnl)
        .def_readwrite("unrealized_pnl", &Metrics::unrealized_pnl)
        .def_readwrite("win_rate", &Metrics::win_rate)
        .def_readwrite("maximum_drawdown", &Metrics::maximum_drawdown)
        .def_readwrite("profit_factor", &Metrics::profit_factor)
        .def_readwrite("expectancy_money", &Metrics::expectancy_money)
        .def_readwrite("expectancy_r", &Metrics::expectancy_r)
        .def_readwrite("amount_of_orders", &Metrics::amount_of_orders)
        .def_readwrite("amount_of_signals", &Metrics::amount_of_signals)
        .def_readwrite("amount_of_executed_orders", &Metrics::amount_of_executed_orders);
}
