/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** NetworkManager.hpp
*/

#pragma once

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <unordered_map>

/**
 * @class NetworkManager
 * @brief Manages network communication with the Zappy server
 *
 * This class handles the connection to the server and implements the GUI protocol.
 */
class NetworkManager {
public:
    /**
     * @brief Constructor for NetworkManager
     */
    NetworkManager();

    /**
     * @brief Destructor for NetworkManager
     */
    ~NetworkManager();

    /**
     * @brief Connects to the Zappy server
     * @param hostname Server hostname or IP address
     * @param port Server port
     * @return True if connection successful
     */
    bool connect(const std::string& hostname, int port);

    /**
     * @brief Disconnects from the server
     */
    void disconnect();

    /**
     * @brief Checks if connected to the server
     * @return True if connected
     */
    bool isConnected() const;

    /**
     * @brief Processes all available server messages
     */
    void update();

    /**
     * @brief Gets the map size
     * @param width Reference to store width
     * @param height Reference to store height
     * @return True if map size was successfully received
     */
    bool getMapSize(int& width, int& height);

    /**
     * @brief Request tile content at specific coordinates
     * @param x X coordinate
     * @param y Y coordinate
     */
    void requestTileContent(int x, int y);

    /**
     * @brief Request content of the entire map
     */
    void requestMapContent();

    /**
     * @brief Request team names
     */
    void requestTeamNames();

    /**
     * @brief Request time unit value
     */
    void requestTimeUnit();

    /**
     * @brief Set the time unit value
     * @param timeUnit New time unit value
     */
    void setTimeUnit(int timeUnit);

    /**
     * @brief Sends a command to the server
     * @param command Command to send
     * @return True if send successful
     */
    bool sendCommand(const std::string& command);

    /**
     * @brief Registers callback functions for different server messages
     * @param type Message type
     * @param callback Function to call when message is received
     */
    template<typename Func>
    void registerCallback(const std::string& type, Func callback) {
        callbacks[type] = callback;
    }

    /**
     * @brief Get the last responses received from the server
     * @return Vector of recent response messages
     */
    std::vector<std::string> getLastResponses();

private:
    int socketFd;                              ///< Socket file descriptor
    bool connected;                            ///< Connection state
    std::string buffer;                        ///< Receive buffer
    std::thread networkThread;                 ///< Thread for network operations
    bool running;                              ///< Thread control flag
    std::mutex mutex;                          ///< Mutex for thread safety
    std::condition_variable condition;         ///< Condition variable for signaling
    std::queue<std::string> messageQueue;      ///< Queue for received messages

    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> callbacks;

    typedef std::function<void(const std::vector<std::string>&)> Callback;

    /**
     * @brief Network thread function
     */
    void networkLoop();

    /**
     * @brief Processes a received message
     * @param message The message to process
     */
    void processMessage(const std::string& message);

    /**
     * @brief Parses a message into command and arguments
     * @param message The message to parse
     * @param command Output for the command
     * @param args Output for the arguments
     */
    void parseMessage(const std::string& message, std::string& command, std::vector<std::string>& args);

    std::vector<std::string> lastResponses;  // Store recent responses for retrieval
    std::mutex responseMutex;                // Mutex for thread-safe access to responses
};
