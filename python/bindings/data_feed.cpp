#include <pybind11/pybind11.h>

#include "backtester/data_feed/DataFeed.hpp"
#include "backtester/models/Instrument.hpp"

namespace py = pybind11;

void bind_data_feed(py::module_ &m) {
    py::class_<DataFeed>(m, "DataFeed")
        .def(py::init<const std::string &, const Instrument &>(), py::arg("filename"),
             py::arg("instrument"));
}
