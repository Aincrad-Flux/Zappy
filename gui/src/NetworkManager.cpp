/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** NetworkManager.cpp
*/

#include "NetworkManager.hpp"
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
}

NetworkManager::~NetworkManager()
{
    disconnect();
}

bool NetworkManager::connect(const std::string& hostname, int port)
{
    if (connected) {
        std::cerr << "Already connected to server" << std::endl;
        return false;
    }

    struct addrinfo hints, *serverInfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::string portStr = std::to_string(port);
    int rv;
    if ((rv = getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &serverInfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return false;
    }

    for(p = serverInfo; p != NULL; p = p->ai_next) {
        if ((socketFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "socket: " << strerror(errno) << std::endl;
            continue;
        }

        if (::connect(socketFd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketFd);
            std::cerr << "connect: " << strerror(errno) << std::endl;
            continue;
        }

        break;
    }

    if (p == NULL) {
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }

    freeaddrinfo(serverInfo);

    int flags = fcntl(socketFd, F_GETFL, 0);
    fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);

    connected = true;
    running = true;

    networkThread = std::thread(&NetworkManager::networkLoop, this);

    std::cout << "Connected to server at " << hostname << ":" << port << std::endl;
    return true;
}

void NetworkManager::disconnect()
{
    if (!connected)
        return;

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

bool NetworkManager::getMapSize(int& width, int& height)
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
        std::cerr << "Not connected to server" << std::endl;
        return false;
    }

    int bytesSent = send(socketFd, command.c_str(), command.length(), 0);
    if (bytesSent <= 0) {
        std::cerr << "Failed to send command: " << command;
        if (bytesSent < 0)
            std::cerr << " (Error: " << strerror(errno) << ")";
        std::cerr << std::endl;
        return false;
    }

    return true;
}

void NetworkManager::networkLoop()
{
    const int bufferSize = 4096;
    char tempBuffer[bufferSize];

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
                }
                pos = buffer.find('\n');
            }
        } else if (bytesReceived == 0) {
            std::cerr << "Connection closed by server" << std::endl;
            connected = false;
            break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "recv error: " << strerror(errno) << std::endl;
                connected = false;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void NetworkManager::processMessage(const std::string& message)
{
    std::string command;
    std::vector<std::string> args;
    parseMessage(message, command, args);
    auto it = callbacks.find(command);

    if (it != callbacks.end()) {
        it->second(args);
    } else {
        std::cout << "Unhandled server message: " << message << std::endl;
    }
}

void NetworkManager::parseMessage(const std::string& message, std::string& command, std::vector<std::string>& args)
{
    std::istringstream iss(message);
    iss >> command;
    std::string arg;

    while (iss >> arg) {
        args.push_back(arg);
    }
}

void NetworkManager::registerCallback(const std::string& command, Callback callback)
{
    callbacks[command] = callback;
}
