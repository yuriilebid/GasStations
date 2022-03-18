#include "logger.h"
#include <dirent.h>
#include <sys/stat.h>

void createDir(const std::string& dbDirName) {
    DIR* dir;
    struct dirent *entry;
    bool directoryFound = false;

    dir = opendir(".");
    if (!dir) {
        perror("diropen");
        exit(1);
    }
    while ( (entry = readdir(dir)) != nullptr && !directoryFound) {
        if(entry->d_name == dbDirName) {
            directoryFound = true;
        }
    }
    if(!directoryFound) {
        LogPrintf(spdlog::level::warn, "|PTS| Directory |\"Logs\"| do not exists. Creating new one");
        int check = mkdir(dbDirName.c_str(), 0777);
        if(check != 0) {
            LogPrintf(spdlog::level::err, "|PTS| Cannot create directory |\"Logs\"|");
            exit(1);
        }
        else {
            LogPrintf(spdlog::level::info, "|PTS| Directory created successfully");
        }
    }
}

int LogOpen(const std::string &path, bool debug, bool syslog) {
    try {
        std::vector<spdlog::sink_ptr> sinks;
        std::vector<spdlog::sink_ptr> fileSinks;
        std::vector<spdlog::sink_ptr> consoleSinks;
        std::vector<spdlog::sink_ptr> sourceTraceSinks;

        if (syslog) {
            sinks.push_back(std::make_shared<spdlog::sinks::syslog_sink_mt>("pts", LOG_PID, LOG_USER, true));
            fileSinks.push_back(std::make_shared<spdlog::sinks::syslog_sink_mt>("pts", LOG_PID, LOG_USER, true));
            consoleSinks.push_back(std::make_shared<spdlog::sinks::syslog_sink_mt>("pts", LOG_PID, LOG_USER, true));
            sourceTraceSinks.push_back(std::make_shared<spdlog::sinks::syslog_sink_mt>("pts", LOG_PID, LOG_USER, true));
        } else {
            createDir("logs");
            createDir("traces");
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            consoleSinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
//            sourceTraceSinks.push_back(std::)
            auto traceFile = std::make_shared<spdlog::sinks::daily_file_sink_mt>("./traces/", 23, 59);
            auto dailyFile = std::make_shared<spdlog::sinks::daily_file_sink_mt>("./logs/", 23, 59);
            sinks.push_back(dailyFile);
            fileSinks.push_back(dailyFile);
            sourceTraceSinks.push_back(traceFile);
        }

        auto logger = std::make_shared<spdlog::logger>(spdlog::logger("main", sinks.begin(), sinks.end()));
        auto fileLogger = std::make_shared<spdlog::logger>(spdlog::logger("file", fileSinks.begin(), fileSinks.end()));
        auto consoleLogger = std::make_shared<spdlog::logger>(spdlog::logger("unique", consoleSinks.begin(), consoleSinks.end()));
        auto traceLogger = std::make_shared<spdlog::logger>(spdlog::logger("trace", sourceTraceSinks.begin(), sourceTraceSinks.end()));
        logger->set_level(debug ? spdlog::level::trace : spdlog::level::info);
        logger->flush_on(debug ? spdlog::level::trace : spdlog::level::info);
        fileLogger->set_level(spdlog::level::trace);
        fileLogger->flush_on(spdlog::level::trace);
        consoleLogger->set_level(debug ? spdlog::level::trace : spdlog::level::info);
        consoleLogger->flush_on(debug ? spdlog::level::trace : spdlog::level::info);
        traceLogger->set_level(debug ? spdlog::level::trace : spdlog::level::info);
        traceLogger->flush_on(debug ? spdlog::level::trace : spdlog::level::info);

        spdlog::register_logger(logger);
        spdlog::register_logger(fileLogger);
        spdlog::register_logger(consoleLogger);
        spdlog::register_logger(traceLogger);
        return 0;
    }
    catch (const spdlog::spdlog_ex &e) {
        std::cout << "Error opening log: " << e.what() << std::endl;
        return -1;
    }
}

