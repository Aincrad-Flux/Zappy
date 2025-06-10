/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** main.cpp
*/

#include "Game.hpp"
#include "Logger.hpp"
#include <iostream>
#include <string>
#include <cstring>

void printUsage()
{
    std::cout << "USAGE: ./zappy_gui -p port -h machine" << std::endl;
    std::cout << "option description" << std::endl;
    std::cout << "-p port        port number" << std::endl;
    std::cout << "-h machine     hostname of the server" << std::endl;
}

int main(int argc, char* argv[])
{
    Logger::getInstance().init("zappy_gui.log", false);
    Logger::getInstance().info("Zappy GUI starting up");

    if (argc > 1 && (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-help") == 0)) {
        Logger::getInstance().info("Help requested, displaying usage information");
        printUsage();
        return 0;
    }

    std::string hostname = "";
    int port = 0;

    Logger::getInstance().info("Parsing command line arguments");

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            try {
                port = std::stoi(argv[i + 1]);
                Logger::getInstance().info("Port set to: " + std::to_string(port));
                i++; // Skip the next argument as it's the port value
            } catch (const std::exception& e) {
                std::string errorMsg = "Error: Invalid port number: " + std::string(argv[i + 1]);
                Logger::getInstance().error(errorMsg);
                std::cerr << errorMsg << std::endl;
                printUsage();
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            hostname = argv[i + 1];
            Logger::getInstance().info("Hostname set to: " + hostname);
            i++; // Skip the next argument as it's the hostname
        } else {
            std::string errorMsg = "Unknown option: " + std::string(argv[i]);
            Logger::getInstance().error(errorMsg);
            std::cerr << errorMsg << std::endl;
            printUsage();
            return 1;
        }
    }
    if ((hostname.empty() && port != 0) || (!hostname.empty() && port == 0)) {
        std::string errorMsg = "Error: Both hostname and port must be provided for server connection";
        Logger::getInstance().error(errorMsg);
        std::cerr << errorMsg << std::endl;
        printUsage();
        return 1;
    }

    try {
        if (!hostname.empty() && port != 0) {
            std::string connectMsg = "Connecting to server at " + hostname + ":" + std::to_string(port);
            Logger::getInstance().info(connectMsg);
            Game game(1600, 900, hostname, port);
            game.run();
        } else {
            Logger::getInstance().info("Starting in offline mode");
            Game game(1600, 900);
            game.run();
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Error: " + std::string(e.what());
        Logger::getInstance().error(errorMsg);
        return 1;
    }

    Logger::getInstance().info("Zappy GUI shutting down normally");

    return 0;
}