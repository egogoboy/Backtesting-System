#pragma once

#include "backtester/config/ExecutionConfig.hpp"
#include "backtester/config/PortfolioConfig.hpp"
#include "backtester/config/RiskManagerConfig.hpp"
#include "backtester/core/EventQueue.hpp"
#include "backtester/data_feed/DataFeed.hpp"
#include "backtester/execution_handler/ExecutionHandler.hpp"
#include "backtester/metric_calculator/MetricCalculator.hpp"
#include "backtester/models/Metrics.hpp"
#include "backtester/portfolio/Portfolio.hpp"
#include "backtester/risk_manager/RiskManager.hpp"
#include "backtester/strategy/Strategy.hpp"
#include <chrono>
#include <functional>

class Engine {
  public:
    Engine(const std::shared_ptr<Strategy> &strategy, DataFeed &data_feed,
           PortfolioConfig &portfolio_config, RiskManagerConfig &risk_manager_config,
           ExecutionConfig &execution_config)
        : start_time_{std::chrono::system_clock::now()}, strategy_{strategy}, data_feed_(data_feed),
          portfolio_(portfolio_config),
          risk_manager_(risk_manager_config, next_step_event_queue_, portfolio_,
                        data_feed_.get().get_current_market_data()),
          execution_handler_(execution_config, event_queue_, portfolio_,
                             data_feed_.get().get_current_market_data()),
          metric_calculator_(portfolio_) {
        strategy_->set_event_queue(next_step_event_queue_);
    }

    Metrics run();

  private:
    std::chrono::system_clock::time_point start_time_;
    std::shared_ptr<Strategy> strategy_;
    std::reference_wrapper<DataFeed> data_feed_;
    Portfolio portfolio_;
    RiskManager risk_manager_;
    ExecutionHandler execution_handler_;
    EventQueue event_queue_;
    EventQueue next_step_event_queue_;
    MetricCalculator metric_calculator_;
};
