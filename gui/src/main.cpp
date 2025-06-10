/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** main.cpp
*/

#include "Game.hpp"
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
    if (argc > 1 && (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-help") == 0)) {
        printUsage();
        return 0;
    }

    std::string hostname = "";
    int port = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            try {
                port = std::stoi(argv[i + 1]);
                i++; // Skip the next argument as it's the port value
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid port number" << std::endl;
                printUsage();
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            hostname = argv[i + 1];
            i++; // Skip the next argument as it's the hostname
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            printUsage();
            return 1;
        }
    }
    if ((hostname.empty() && port != 0) || (!hostname.empty() && port == 0)) {
        std::cerr << "Error: Both hostname and port must be provided for server connection" << std::endl;
        printUsage();
        return 1;
    }

    try {
        if (!hostname.empty() && port != 0) {
            std::cout << "Connecting to server at " << hostname << ":" << port << std::endl;
            Game game(1600, 900, hostname, port);
            game.run();
        } else {
            std::cout << "Starting in offline mode" << std::endl;
            Game game(1600, 900);
            game.run();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}