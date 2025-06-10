/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** Logger.cpp
*/

#include "Logger.hpp"
#include <sstream>

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger() : initialized(false) {}

Logger::~Logger()
{
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::init(const std::string& logFilePath)
{
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (initialized) {
        return;
    }
    
    if (!logFilePath.empty()) {
        logFile.open(logFilePath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Warning: Could not open log file: " << logFilePath << std::endl;
        }
    }
    
    initialized = true;
}

void Logger::log(Level level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(logMutex);
    
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // Format timestamp
    std::stringstream timestamp;
    timestamp << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    timestamp << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    // Format log entry
    std::string logEntry = "[" + timestamp.str() + "] [" + levelToString(level) + "] " + message;
    
    // Output to console
    std::cout << logEntry << std::endl;
    
    // Output to file if available
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
        logFile.flush();
    }
}

void Logger::info(const std::string& message)
{
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message)
{
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message)
{
    log(Level::ERROR, message);
}

void Logger::debug(const std::string& message)
{
    log(Level::DEBUG, message);
}

void Logger::network(const std::string& message)
{
    log(Level::NETWORK, message);
}

std::string Logger::levelToString(Level level)
{
    switch (level) {
        case Level::INFO:
            return "INFO";
        case Level::WARNING:
            return "WARNING";
        case Level::ERROR:
            return "ERROR";
        case Level::DEBUG:
            return "DEBUG";
        case Level::NETWORK:
            return "NETWORK";
        default:
            return "UNKNOWN";
    }
}