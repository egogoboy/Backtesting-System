#include "backtester/events/MarketEvent.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_events(py::module_ &m) {
    py::class_<MarketEvent>(m, "MarketEvent")
        .def_property_readonly("data", &MarketEvent::get_data,
                               py::return_value_policy::reference_internal);
}
