#include "backtester/core/Engine.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_engine(py::module_ &m) {
    py::class_<Engine>(m, "Engine")
        .def(py::init<const std::shared_ptr<Strategy> &, DataFeed &, PortfolioConfig &,
                      RiskManagerConfig &, ExecutionConfig &>())
        .def("run", &Engine::run);
}
