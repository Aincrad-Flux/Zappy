#include "Game.hpp"
#include <iostream>
#include <cstring>

void printUsage() {
    std::cout << "USAGE: ./zappy_gui -p port -h machine" << std::endl;
    std::cout << "option description" << std::endl;
    std::cout << "-p port        port number" << std::endl;
    std::cout << "-h machine     hostname of the server" << std::endl;
}

int main(int argc, char* argv[]) {
    // Pour l'instant, on ignore les arguments et on lance en mode démo
    // Plus tard, on ajoutera la connexion réseau

    if (argc > 1 && (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0)) {
        printUsage();
        return 0;
    }

    try {
        Game game(1600, 900);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}