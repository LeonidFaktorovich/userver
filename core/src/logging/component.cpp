#include <logging/component.hpp>

#include <chrono>
#include <stdexcept>

// this header must be included before any spdlog headers
// to override spdlog's level names
#include <boost/filesystem/operations.hpp>
#include <logging/spdlog.hpp>

#include <fmt/format.h>

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <engine/async.hpp>
#include <engine/sleep.hpp>
#include <logging/log.hpp>
#include <logging/logger.hpp>
#include <logging/reopening_file_sink.hpp>
#include <utils/thread_name.hpp>

#include "config.hpp"

namespace components {
namespace {

constexpr std::chrono::seconds kDefaultFlushInterval{2};

}  // namespace

Logging::Logging(const ComponentConfig& config,
                 const ComponentContext& context) {
  const auto fs_task_processor_name =
      config["fs-task-processor"].As<std::string>();
  fs_task_processor_ = &context.GetTaskProcessor(fs_task_processor_name);

  const auto loggers = config["loggers"];

  for (const auto& [logger_name, logger_yaml] : Items(loggers)) {
    const bool is_default_logger = logger_name == "default";

    const auto logger_config = logger_yaml.As<logging::LoggerConfig>();
    auto logger = CreateLogger(logger_name, logger_config, is_default_logger);

    logger->set_level(
        static_cast<spdlog::level::level_enum>(logger_config.level));
    logger->set_pattern(logger_config.pattern);
    logger->flush_on(
        static_cast<spdlog::level::level_enum>(logger_config.flush_level));

    if (is_default_logger) {
      logging::SetDefaultLogger(logger);
    } else {
      auto insertion_result = loggers_.emplace(logger_name, std::move(logger));
      if (!insertion_result.second) {
        throw std::runtime_error("duplicate logger '" +
                                 insertion_result.first->first + '\'');
      }
    }
  }
  flush_task_.Start("log_flusher",
                    utils::PeriodicTask::Settings(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            kDefaultFlushInterval),
                        {}, logging::Level::kTrace),
                    GetTaskFunction());
}

Logging::~Logging() { flush_task_.Stop(); }

logging::LoggerPtr Logging::GetLogger(const std::string& name) {
  auto it = loggers_.find(name);
  if (it == loggers_.end()) {
    throw std::runtime_error("logger '" + name + "' not found");
  }
  return it->second;
}

namespace {

void ReopenAll(std::vector<spdlog::sink_ptr>& sinks) {
  for (const auto& s : sinks) {
    auto reop = std::dynamic_pointer_cast<logging::ReopeningFileSinkMT>(s);
    if (!reop) {
      continue;
    }

    try {
      reop->Reopen(/* truncate = */ false);
    } catch (const std::exception& e) {
      LOG_ERROR() << "Exception on log reopen: " << e;
    }
  }
};

void CreateLogDirectory(const std::string& logger_name,
                        const std::string& file_path) {
  try {
    const auto dirname = boost::filesystem::path(file_path).parent_path();
    boost::filesystem::create_directories(dirname);
  } catch (const std::exception& e) {
    auto msg = "Failed to create directory for log file of logger '" +
               logger_name + "': " + e.what();
    LOG_ERROR() << msg;
    throw std::runtime_error(msg);
  }
}

}  // namespace

void Logging::OnLogRotate() {
  std::vector<engine::TaskWithResult<void>> tasks;
  tasks.reserve(loggers_.size() + 1);

  // this must be a copy as the default logger may change
  auto default_logger = logging::DefaultLogger();
  tasks.push_back(engine::impl::CriticalAsync(
      *fs_task_processor_, ReopenAll, std::ref(default_logger->sinks())));

  for (const auto& item : loggers_) {
    tasks.push_back(engine::impl::CriticalAsync(
        *fs_task_processor_, ReopenAll, std::ref(item.second->sinks())));
  }

  for (auto& task : tasks) {
    try {
      task.BlockingWait();
      task.Get();
    } catch (const std::exception& e) {
      LOG_ERROR() << "Exception escaped ReopenAll: " << e;
    }
  }
}

void Logging::FlushLogs() {
  logging::DefaultLogger()->flush();
  for (auto& item : loggers_) {
    item.second->flush();
  }
}

std::shared_ptr<spdlog::logger> Logging::CreateLogger(
    const std::string& logger_name, const logging::LoggerConfig& logger_config,
    bool is_default_logger) {
  if (logger_config.file_path == "@null")
    return logging::MakeNullLogger(logger_name);
  if (logger_config.file_path == "@stderr")
    return logging::MakeStderrLogger(logger_name, logger_config.level);

  auto overflow_policy = spdlog::async_overflow_policy::overrun_oldest;
  if (logger_config.queue_overflow_behavior ==
      logging::LoggerConfig::QueueOveflowBehavior::kBlock) {
    overflow_policy = spdlog::async_overflow_policy::block;
  }

  CreateLogDirectory(logger_name, logger_config.file_path);

  auto file_sink =
      std::make_shared<logging::ReopeningFileSinkMT>(logger_config.file_path);
  std::shared_ptr<spdlog::details::thread_pool> tp;

  auto old_thread_name = utils::GetCurrentThreadName();
  utils::SetCurrentThreadName("log/" + logger_name);

  if (is_default_logger) {
    spdlog::init_thread_pool(logger_config.message_queue_size,
                             logger_config.thread_pool_size);
    tp = spdlog::thread_pool();

  } else {
    tp = std::make_shared<spdlog::details::thread_pool>(
        logger_config.message_queue_size, logger_config.thread_pool_size);
    thread_pools_.push_back(tp);
  }

  utils::SetCurrentThreadName(old_thread_name);

  return std::make_shared<spdlog::async_logger>(
      logger_name, std::move(file_sink), tp, overflow_policy);
}

}  // namespace components
