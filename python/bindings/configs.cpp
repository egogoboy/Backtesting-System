#include <pybind11/pybind11.h>

#include "backtester/config/ExecutionConfig.hpp"
#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/config/RiskManagerConfig.hpp"

namespace py = pybind11;

void bind_configs(py::module_ &m) {
    py::class_<ExecutionConfig>(m, "ExecutionConfig")
        .def(py::init<double, double>(), py::arg("margin_rate"), py::arg("maintenance_margin_rate"))
        .def_readwrite("margin_rate", &ExecutionConfig::margin_rate)
        .def_readwrite("maintenance_margin_rate", &ExecutionConfig::maintenance_margin_rate);

    py::class_<PortfolioConfig>(m, "PortfolioConfig")
        .def(py::init<uint32_t, double>(), py::arg("account_id"), py::arg("account_balance"))
        .def_readwrite("account_id", &PortfolioConfig::account_id)
        .def_readwrite("account_balance", &PortfolioConfig::account_balance);

    py::class_<RiskManagerConfig>(m, "RiskManagerConfig")
        .def(py::init<double, double, double>(), py::arg("risk_per_trade"),
             py::arg("total_account_risk"), py::arg("sl_tp_ratio"))
        .def_readwrite("risk_per_trade", &RiskManagerConfig::risk_per_trade)
        .def_readwrite("total_account_risk", &RiskManagerConfig::total_account_risk)
        .def_readwrite("sl_tp_ratio", &RiskManagerConfig::sl_tp_ratio);
}
