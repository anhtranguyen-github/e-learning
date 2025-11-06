#include "client/network.h"
#include "client/ui.h"
#include "common/logger.h"
#include <iostream>
#include <csignal>

int main(int argc, char* argv[]) {
    // Initialize logger
    logger::initClientLogger();

    std::string host = "127.0.0.1";
    int port = 8080;

    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = std::atoi(argv[2]);
    }

    if (logger::clientLogger) {
        logger::clientLogger->info("Starting TCP Learning App Client");
        logger::clientLogger->info("Target server: " + host + ":" + std::to_string(port));
    }

    client::NetworkClient network(host, port);
    client::UI ui(network);

    // Handle Ctrl+C gracefully
    signal(SIGINT, [](int) {
        std::cout << "\n\nShutting down client..." << std::endl;
        exit(0);
    });

    ui.run();

    if (logger::clientLogger) {
        logger::clientLogger->info("Client terminated");
    }

    return 0;
}
