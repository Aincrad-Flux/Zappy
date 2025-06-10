/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** zappy_console.cpp - Simple command line tool for Zappy server communication
*/

#include "NetworkManager.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

void printHelp()
{
    std::cout << "\n=== Zappy Console Client Help ===" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  msz                - Get map size" << std::endl;
    std::cout << "  bct X Y            - Get content of tile at (X,Y)" << std::endl;
    std::cout << "  mct                - Get content of all tiles" << std::endl;
    std::cout << "  tna                - Get team names" << std::endl;
    std::cout << "  ppo #n             - Get position of player #n" << std::endl;
    std::cout << "  plv #n             - Get level of player #n" << std::endl;
    std::cout << "  pin #n             - Get inventory of player #n" << std::endl;
    std::cout << "  sgt                - Get server time unit" << std::endl;
    std::cout << "  sst T              - Set server time unit to T" << std::endl;
    std::cout << "  quit/exit          - Exit the program" << std::endl;
    std::cout << "  help               - Show this help message" << std::endl;
    std::cout << "================================\n" << std::endl;
}

int main(int argc, char* argv[])
{
    // Parse command line arguments
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
        return 1;
    }

    std::string hostname = argv[1];
    int port;

    try {
        port = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid port number" << std::endl;
        return 1;
    }

    // Initialize logger (with console output disabled)
    Logger::getInstance().init("zappy_console.log", false);
    Logger::getInstance().info("Zappy Console Client starting up");

    // Create network manager
    NetworkManager networkManager;

    // Connect to server
    std::cout << "Connecting to server at " << hostname << ":" << port << "..." << std::endl;

    if (!networkManager.connect(hostname, port)) {
        std::cerr << "Failed to connect to server at " << hostname << ":" << port << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;
    printHelp();

    // Setup a generic message handler
    networkManager.registerCallback("", [](const std::vector<std::string>& args) {
        // This is a fallback and should not be called with the current implementation
    });

    std::string input;
    bool running = true;

    // Main command loop
    while (running) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "quit" || input == "exit") {
            running = false;
            continue;
        }

        if (input == "help") {
            printHelp();
            continue;
        }

        if (!input.empty()) {
            // Add newline character required by Zappy protocol
            std::string command = input + "\n";

            if (networkManager.sendCommand(command)) {
                std::cout << "Command sent: " << input << std::endl;

                // Give the server time to respond
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // Process any received messages
                networkManager.update();

                // Display responses from the server history
                auto responses = networkManager.getLastResponses();
                if (responses.empty()) {
                    std::cout << "No response received within timeout period" << std::endl;
                } else {
                    std::cout << "Server responses:" << std::endl;
                    for (const auto& resp : responses) {
                        std::cout << "  " << resp << std::endl;
                    }
                }
            } else {
                std::cout << "Failed to send command" << std::endl;
            }
        }
    }

    // Disconnect from server
    networkManager.disconnect();
    std::cout << "Disconnected from server. Goodbye!" << std::endl;

    return 0;
}