#include "router.h"
#include <unistd.h>

Router::Router(int listenPort, const std::string& forwardIP, int forwardPort) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Listening address
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = INADDR_ANY;
    listenAddr.sin_port = htons(listenPort);

    if (bind(sockfd, (struct sockaddr*)&listenAddr, sizeof(listenAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Forwarding address
    forwardAddr.sin_family = AF_INET;
    forwardAddr.sin_port = htons(forwardPort);
    inet_pton(AF_INET, forwardIP.c_str(), &forwardAddr.sin_addr);
}

void Router::startRouting() {
    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    std::cout << "Router listening for packets...\n";

    while (true) {
        int recvLen = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&clientAddr, &clientLen);
        if (recvLen > 0) {
            std::cout << "Packet received, forwarding...\n";
            sendto(sockfd, buffer, recvLen, 0, (struct sockaddr*)&forwardAddr, sizeof(forwardAddr));
        }
    }
}
