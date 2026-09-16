#include "backtester/strategy/Strategy.hpp"
#include "backtester/strategy/PyStrategy.hpp"
#include <memory>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_strategy(py::module_ &m) {
    py::class_<Strategy, PyStrategy, std::shared_ptr<Strategy>>(m, "Strategy")
        .def(py::init<>())
        .def("on_market", &Strategy::on_market, py::arg("event"))
        .def("send_signal", &PyStrategy::send_signal);
}
