#ifndef ROUTER_H
#define ROUTER_H

#include <bits/stdc++.h>
#include <arpa/inet.h>
#include "packet.h"
using namespace std;
class Router {
public:
    Router(int listenPort, const string& forwardIP, int forwardPort);
    void startRouting();

private:
    int sockfd;
    struct sockaddr_in listenAddr, forwardAddr;
};

#endif // ROUTER_H
