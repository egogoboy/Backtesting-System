#include "backtester/enums/Direction.hpp"
#include "backtester/enums/OrderType.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_enums(py::module_ &m) {
    py::enum_<Direction>(m, "Direction")
        .value("LONG", Direction::LONG)
        .value("SHORT", Direction::SHORT);

    py::enum_<OrderType>(m, "SignalType")
        .value("MARKET", OrderType::MARKET)
        .value("LIMIT", OrderType::LIMIT)
        .value("STOP", OrderType::STOP);
}
