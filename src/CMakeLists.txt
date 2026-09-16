add_library(backtester_core)

set_target_properties(backtester_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(backtester_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_sources(backtester_core
    PRIVATE
        core/Engine.cpp

        portfolio/Portfolio.cpp
        risk_manager/RiskManager.cpp
        execution_handler/ExecutionHandler.cpp
        metric_calculator/MetricCalculator.cpp

        models/Order.cpp

        utils/CSVFileLoader.cpp
        utils/PnLCalculator.cpp
)
