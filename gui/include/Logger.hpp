/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Logger.hpp
*/

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>

/**
 * @class Logger
 * @brief Provides logging functionality with different log levels
 *
 * This class implements a thread-safe logger that can output messages
 * with timestamps and log levels to console and/or file.
 */
class Logger {
public:
    /**
     * @brief Log levels
     */
    enum class Level {
        INFO,
        WARNING,
        ERROR,
        DEBUG,
        NETWORK
    };

    /**
     * @brief Get the singleton instance of the Logger
     * @return Reference to the Logger instance
     */
    static Logger& getInstance();

    /**
     * @brief Initialize the logger with a file path
     * @param logFilePath Path to the log file (empty for console-only logging)
     * @param consoleOutput Whether to output logs to console (default: true)
     */
    void init(const std::string& logFilePath = "", bool consoleOutput = true);

    /**
     * @brief Enable or disable console output
     * @param enable True to enable console output, false to disable
     */
    void setConsoleOutput(bool enable);

    /**
     * @brief Log a message with specified level
     * @param level Log level
     * @param message Message to log
     */
    void log(Level level, const std::string& message);

    /**
     * @brief Log an info message
     * @param message Message to log
     */
    void info(const std::string& message);

    /**
     * @brief Log a warning message
     * @param message Message to log
     */
    void warning(const std::string& message);

    /**
     * @brief Log an error message
     * @param message Message to log
     */
    void error(const std::string& message);

    /**
     * @brief Log a debug message
     * @param message Message to log
     */
    void debug(const std::string& message);

    /**
     * @brief Log a network message
     * @param message Message to log
     */
    void network(const std::string& message);

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    Logger();

    /**
     * @brief Destructor
     */
    ~Logger();

    /**
     * @brief Get string representation of log level
     * @param level Log level
     * @return String representation of log level
     */
    std::string levelToString(Level level);

    std::mutex logMutex;
    std::ofstream logFile;
    bool initialized;
    bool enableConsole;
};
