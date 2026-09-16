#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_configs(py::module_ &m);
void bind_enums(py::module_ &m);
void bind_models(py::module_ &m);
void bind_data_feed(py::module_ &m);
void bind_events(py::module_ &m);
void bind_strategy(py::module_ &m);
void bind_engine(py::module_ &m);

PYBIND11_MODULE(_backtester, m) {
    m.doc() = "Python bindings for the backtesting engine";

    bind_configs(m);
    bind_enums(m);
    bind_models(m);
    bind_data_feed(m);
    bind_events(m);
    bind_strategy(m);
    bind_engine(m);
}
