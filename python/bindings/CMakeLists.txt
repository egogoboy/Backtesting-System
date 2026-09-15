set(BINDINGS_SOURCES
    bindings.cpp
    configs.cpp
    enums.cpp
    models.cpp
)

pybind11_add_module(_backtester ${BINDINGS_SOURCES})

target_link_libraries(_backtester
    PRIVATE
        backtester_core
)

target_include_directories(_backtester
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
