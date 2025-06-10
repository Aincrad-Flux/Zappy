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

    // Wait a moment to receive the initial WELCOME message
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    networkManager.update();

    // Wait for WELCOME message from server
    std::cout << "Waiting for server welcome message..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    networkManager.update();

    // Send GRAPHIC command to identify as a graphical client
    std::cout << "Identifying as graphical client..." << std::endl;
    if (networkManager.sendCommand("GRAPHIC\n")) {
        // Give the server time to process the GRAPHIC command and send initial data
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        networkManager.update();
        std::cout << "Sent GRAPHIC identification" << std::endl;

        // Process any received messages from the server after identification
        auto initialResponses = networkManager.getLastResponses();
        if (!initialResponses.empty()) {
            std::cout << "Received initial data from server:" << std::endl;
            for (const auto& resp : initialResponses) {
                std::cout << "  " << resp << std::endl;
            }
        }
    } else {
        std::cerr << "Failed to send GRAPHIC identification" << std::endl;
        return 1;
    }

    printHelp();

    // Setup message handlers for different server responses
    networkManager.registerCallback("msz", [](const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            std::cout << "Map size: " << args[0] << " x " << args[1] << std::endl;
        } else {
            std::cout << "Invalid msz response format" << std::endl;
        }
    });

    networkManager.registerCallback("bct", [](const std::vector<std::string>& args) {
        if (args.size() >= 9) {
            std::cout << "Tile (" << args[0] << "," << args[1] << ") content:" << std::endl;
            std::cout << "  Food: " << args[2] << std::endl;
            std::cout << "  Linemate: " << args[3] << std::endl;
            std::cout << "  Deraumere: " << args[4] << std::endl;
            std::cout << "  Sibur: " << args[5] << std::endl;
            std::cout << "  Mendiane: " << args[6] << std::endl;
            std::cout << "  Phiras: " << args[7] << std::endl;
            std::cout << "  Thystame: " << args[8] << std::endl;
        } else {
            std::cout << "Invalid bct response format" << std::endl;
        }
    });

    networkManager.registerCallback("tna", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::cout << "Team name: " << args[0] << std::endl;
        } else {
            std::cout << "Invalid tna response format" << std::endl;
        }
    });

    networkManager.registerCallback("ppo", [](const std::vector<std::string>& args) {
        if (args.size() >= 4) {
            std::cout << "Player #" << args[0] << " position: (" << args[1] << "," << args[2]
                      << "), orientation: " << args[3] << std::endl;
        } else {
            std::cout << "Invalid ppo response format" << std::endl;
        }
    });

    networkManager.registerCallback("plv", [](const std::vector<std::string>& args) {
        if (args.size() >= 2) {
            std::cout << "Player #" << args[0] << " level: " << args[1] << std::endl;
        } else {
            std::cout << "Invalid plv response format" << std::endl;
        }
    });

    networkManager.registerCallback("pin", [](const std::vector<std::string>& args) {
        if (args.size() >= 10) {
            std::cout << "Player #" << args[0] << " inventory:" << std::endl;
            std::cout << "  Position: (" << args[1] << "," << args[2] << ")" << std::endl;
            std::cout << "  Food: " << args[3] << std::endl;
            std::cout << "  Linemate: " << args[4] << std::endl;
            std::cout << "  Deraumere: " << args[5] << std::endl;
            std::cout << "  Sibur: " << args[6] << std::endl;
            std::cout << "  Mendiane: " << args[7] << std::endl;
            std::cout << "  Phiras: " << args[8] << std::endl;
            std::cout << "  Thystame: " << args[9] << std::endl;
        } else {
            std::cout << "Invalid pin response format" << std::endl;
        }
    });

    networkManager.registerCallback("sgt", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::cout << "Server time unit: " << args[0] << std::endl;
        } else {
            std::cout << "Invalid sgt response format" << std::endl;
        }
    });

    networkManager.registerCallback("sst", [](const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::cout << "Server time unit set to: " << args[0] << std::endl;
        } else {
            std::cout << "Invalid sst response format" << std::endl;
        }
    });

    networkManager.registerCallback("ko", [](const std::vector<std::string>& args) {
        std::cout << "Error: Command failed (ko)" << std::endl;
    });

    networkManager.registerCallback("WELCOME", [](const std::vector<std::string>& args) {
        std::cout << "Received welcome message from server" << std::endl;
    });

    // Setup a generic message handler for unrecognized messages
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