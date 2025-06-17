/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** NetworkManager.cpp
*/

#include "NetworkManager.hpp"
#include "Logger.hpp"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>

NetworkManager::NetworkManager()
    : socketFd(-1), connected(false), running(false)
{
    Logger::getInstance().init("zappy_gui_network.log", false);
    Logger::getInstance().info("NetworkManager initialized");
}

NetworkManager::~NetworkManager()
{
    Logger::getInstance().info("NetworkManager shutting down");
    disconnect();
}

bool NetworkManager::connect(const std::string& hostname, int port)
{
    if (connected) {
        Logger::getInstance().warning("Connect attempt failed: Already connected to server");
        std::cerr << "Already connected to server" << std::endl;
        return false;
    }

    Logger::getInstance().info("Attempting to connect to " + hostname + ":" + std::to_string(port));

    struct addrinfo hints, *serverInfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::string portStr = std::to_string(port);
    int rv;
    if ((rv = getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &serverInfo)) != 0) {
        std::string errorMsg = "getaddrinfo: " + std::string(gai_strerror(rv));
        Logger::getInstance().error(errorMsg);
        std::cerr << errorMsg << std::endl;
        return false;
    }

    for(p = serverInfo; p != NULL; p = p->ai_next) {
        if ((socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::string errorMsg = "socket: " + std::string(strerror(errno));
            Logger::getInstance().error(errorMsg);
            std::cerr << errorMsg << std::endl;
            continue;
        }

        if (::connect(socketFd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketFd);
            std::string errorMsg = "connect: " + std::string(strerror(errno));
            Logger::getInstance().error(errorMsg);
            std::cerr << errorMsg << std::endl;
            continue;
        }

        break;
    }

    if (p == NULL) {
        Logger::getInstance().error("Failed to connect to server");
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }

    freeaddrinfo(serverInfo);

    int flags = fcntl(socketFd, F_GETFL, 0);
    fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);

    connected = true;
    running = true;

    networkThread = std::thread(&NetworkManager::networkLoop, this);

    std::string successMsg = "Connected to server at " + hostname + ":" + std::to_string(port);
    Logger::getInstance().info(successMsg);
    std::cout << successMsg << std::endl;
    return true;
}

void NetworkManager::disconnect()
{
    if (!connected)
        return;

    Logger::getInstance().info("Disconnecting from server");
    running = false;

    if (networkThread.joinable()) {
        condition.notify_all();
        networkThread.join();
    }

    if (socketFd != -1) {
        close(socketFd);
        socketFd = -1;
    }

    connected = false;
    Logger::getInstance().info("Disconnected from server");
    std::cout << "Disconnected from server" << std::endl;
}

bool NetworkManager::isConnected() const
{
    return connected;
}

void NetworkManager::update()
{
    std::lock_guard<std::mutex> lock(mutex);

    while (!messageQueue.empty()) {
        std::string message = messageQueue.front();
        messageQueue.pop();
        processMessage(message);
    }
}

bool NetworkManager::getMapSize()
{
    if (!connected) {
        std::cerr << "Not connected to server" << std::endl;
        return false;
    }
    return sendCommand("msz\n");
}

void NetworkManager::requestTileContent(int x, int y)
{
    if (!connected)
        return;

    std::string command = "bct " + std::to_string(x) + " " + std::to_string(y) + "\n";
    sendCommand(command);
}

void NetworkManager::requestMapContent()
{
    if (!connected)
        return;
    sendCommand("mct\n");
}

void NetworkManager::requestTeamNames()
{
    if (!connected)
        return;
    sendCommand("tna\n");
}

void NetworkManager::requestTimeUnit()
{
    if (!connected)
        return;
    sendCommand("sgt\n");
}

void NetworkManager::setTimeUnit(int timeUnit)
{
    if (!connected)
        return;
    std::string command = "sst " + std::to_string(timeUnit) + "\n";
    sendCommand(command);
}

bool NetworkManager::sendCommand(const std::string& command)
{
    if (!connected || socketFd == -1) {
        Logger::getInstance().error("Send command failed: Not connected to server");
        std::cerr << "Not connected to server" << std::endl;
        return false;
    }

    Logger::getInstance().network("Sending command: " + command.substr(0, command.length() - 1)); // Remove trailing newline for log

    int bytesSent = send(socketFd, command.c_str(), command.length(), 0);
    if (bytesSent <= 0) {
        std::string errorMsg = "Failed to send command: " + command.substr(0, command.length() - 1);
        if (bytesSent < 0)
            errorMsg += " (Error: " + std::string(strerror(errno)) + ")";

        Logger::getInstance().error(errorMsg);
        std::cerr << errorMsg << std::endl;
        return false;
    }

    return true;
}

void NetworkManager::networkLoop()
{
    const int bufferSize = 4096;
    char tempBuffer[bufferSize];

    Logger::getInstance().info("Network receive thread started");

    while (running) {
        if (!connected) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        int bytesReceived = recv(socketFd, tempBuffer, bufferSize - 1, 0);

        if (bytesReceived > 0) {
            tempBuffer[bytesReceived] = '\0';
            buffer += tempBuffer;
            size_t pos = buffer.find('\n');
            while (pos != std::string::npos) {
                std::string message = buffer.substr(0, pos);
                buffer = buffer.substr(pos + 1);
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    messageQueue.push(message);
                    Logger::getInstance().network("Received: " + message);
                }
                pos = buffer.find('\n');
            }
        } else if (bytesReceived == 0) {
            Logger::getInstance().error("Connection closed by server");
            std::cerr << "Connection closed by server" << std::endl;
            connected = false;
            break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::string errorMsg = "recv error: " + std::string(strerror(errno));
                Logger::getInstance().error(errorMsg);
                std::cerr << errorMsg << std::endl;
                connected = false;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    Logger::getInstance().info("Network receive thread terminated");
}

void NetworkManager::processMessage(const std::string& message)
{
    // Handle special welcome message
    if (message == "WELCOME") {
        Logger::getInstance().info("Received welcome message from server");
        std::cout << "Received initial welcome from server!" << std::endl;

        // Send GRAPHIC identification
        Logger::getInstance().info("Identifying as graphical client");
        std::cout << "Identifying as graphical client..." << std::endl;
        sendCommand("GRAPHIC\n");

        return;
    }

    std::string command;
    std::vector<std::string> args;
    parseMessage(message, command, args);
    auto it = callbacks.find(command);

    if (it != callbacks.end()) {
        Logger::getInstance().debug("Processing command: " + command);
        it->second(args);
    } else {
        std::string unhandledMsg = "Unhandled server message: " + message;
        Logger::getInstance().warning(unhandledMsg);
        std::cout << unhandledMsg << std::endl;
    }

    // Store the message in the response history
    {
        std::lock_guard<std::mutex> lock(responseMutex);
        lastResponses.push_back(message);
        // Keep only the last 20 responses
        if (lastResponses.size() > 20) {
            lastResponses.erase(lastResponses.begin());
        }
    }
}

std::vector<std::string> NetworkManager::getLastResponses()
{
    std::lock_guard<std::mutex> lock(responseMutex);
    return lastResponses;
}

void NetworkManager::parseMessage(const std::string& message, std::string& command, std::vector<std::string>& args)
{
    command.clear();
    args.clear();

    if (message.empty()) {
        return;
    }

    std::istringstream iss(message);
    iss >> command;
    std::string arg;

    while (iss >> arg) {
        arg.erase(0, arg.find_first_not_of(" \t\n\r\f\v"));
        arg.erase(arg.find_last_not_of(" \t\n\r\f\v") + 1);

        if (!arg.empty()) {
            args.push_back(arg);
        }
    }

    Logger::getInstance().debug("Parsed command: '" + command + "' with " +
                              std::to_string(args.size()) + " arguments");
}

