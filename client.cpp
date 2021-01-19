//
// Created by elena on 19.01.2021.
//

#include "common.h"

int main()
{
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &address.sin_addr)<=0)
    {
        std::cerr << "Invalid address" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    char buffer[BUF_SIZE + 1];
    read(sock , buffer, BUF_SIZE + 1);
    std::cout << "Server response: " << buffer << std::endl;

    while(true)
    {
        std::string s;
        std::cin >> s;

        if(send(sock, s.c_str(), s.size(), 0 ) < 0)
        {
            std::cerr << "Connection is closed" << std::endl;
            break;
        }
    }
}