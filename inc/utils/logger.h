#ifndef PTS2_0_LOGGER_H
#define PTS2_0_LOGGER_H


#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>

int LogOpen(const std::string &path, bool debug, bool syslog);

template<class ...Args>void LogPrintf(spdlog::level::level_enum level, const std::string &format, Args... args) {
    try {
        auto logger = spdlog::get("main");
        if (logger) {
            logger->log(level, format, std::forward<Args>(args)...);
        } else {
            spdlog::log(level, format, std::forward<Args>(args)...);
        }
        return;
    } catch (spdlog::spdlog_ex &e) {
        std::cout << "Error printing to log: " << e.what() << std::endl;
    }
}

template<class ...Args>void LogTrace(spdlog::level::level_enum level, const std::string &format, Args... args) {
    try {
        auto logger = spdlog::get("trace");
        if (logger) {
            logger->log(level, format, std::forward<Args>(args)...);
        } else {
            spdlog::log(level, format, std::forward<Args>(args)...);
        }
        return;
    } catch (spdlog::spdlog_ex &e) {
        std::cout << "Error printing to log: " << e.what() << std::endl;
    }
}

template<class ...Args>void LogConsole(spdlog::level::level_enum level, const std::string &format, Args... args) {
    try {
        auto logger = spdlog::get("unique");
        if (logger) {
            logger->log(level, format, std::forward<Args>(args)...);
        } else {
            spdlog::log(level, format, std::forward<Args>(args)...);
        }
        return;
    } catch (spdlog::spdlog_ex &e) {
        std::cout << "Error printing to log: " << e.what() << std::endl;
    }
}

template<class ...Args>void LogFile(spdlog::level::level_enum level, const std::string &format, Args... args) {
    try {
        auto logger = spdlog::get("file");
        if (logger) {
            logger->log(level, format, std::forward<Args>(args)...);
        } else {
            spdlog::log(level, format, std::forward<Args>(args)...);
        }
        return;
    } catch (spdlog::spdlog_ex &e) {
        std::cout << "Error printing to log: " << e.what() << std::endl;
    }
}

template <typename To, typename From, typename Deleter>
    std::unique_ptr<To, Deleter> dynamic_unique_cast(std::unique_ptr<From, Deleter>&& p) {
        if (To* cast = dynamic_cast<To*>(p.get()))
        {
            std::unique_ptr<To, Deleter> result(cast, std::move(p.get_deleter()));
            p.release();
            return result;
        }
        return std::unique_ptr<To, Deleter>(nullptr); // or throw std::bad_cast() if you prefer
    }

#endif //PTS2_0_LOGGER_H
