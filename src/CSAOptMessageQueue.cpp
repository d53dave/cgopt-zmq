#include "CSAOptMessageQueue.h"

CSAOpt::MessageQueue::MessageQueue(int port) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
//    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink>("csaopt_msqqueue", "log", 1024 * 1024 * 5, 10));
    this->logger = std::make_shared<spdlog::logger>("name", begin(sinks), end(sinks));
    spdlog::register_logger(this->logger);

    this->logger->info("Messagequeue started on port {}", port);
};