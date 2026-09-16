add_executable(unit_tests
    test_main.cpp

    test_csv_file_loader.cpp
    test_pnl_calculator.cpp

    test_signal.cpp
    test_order.cpp
    test_position.cpp

    test_data_feed.cpp
    test_event_queue.cpp
    test_strategy.cpp
    test_portfolio.cpp
    test_risk_manager.cpp
    test_execution_handler.cpp

    test_metrics.cpp

    test_workflow.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        backtester_core
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(unit_tests)
