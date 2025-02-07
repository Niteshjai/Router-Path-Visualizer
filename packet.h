#ifndef PACKET_H
#define PACKET_H

#include <bits/stdc++.h>
using namespace std;
struct Packet {
    char data[1024];  
    int length;

    Packet(const string& message) {
        length = message.length();
        memset(data, 0, sizeof(data));
        memcpy(data, message.c_str(), length);
    }
};

#endif 
