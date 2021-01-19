#include "common.h"

#include <sys/time.h>
#include <string>
#include <fstream>
#include <vector>

void writeToFile(char * buffer, int sd)
{
    std::ofstream out;
    out.open("logs.txt", std::ios_base::app);
    printf("%d - %s \n", sd, buffer );
    out << sd << " - " << buffer << std::endl;
    out.close();
}

int main()
{
    std::vector<std::pair<int, int>> client_socket;

    int master_socket;
    if((master_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    {
        std::cerr << "Master socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    int addrlen = sizeof(address);

    if (bind(master_socket, (struct sockaddr *)&address, addrlen)<0)
    {
        std::cerr << "Master socket bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(master_socket, MAX_CLIENTS) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started" << std::endl;

    fd_set readfds;

    while(true)
    {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        int max_sd = master_socket;

        for (auto client : client_socket)
        {
            FD_SET(client.first, &readfds);
            if(client.first > max_sd)
                max_sd = client.first;
        }

        int activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR))
        {
            std::cerr << "Error on select" << std::endl;
        }

        if (FD_ISSET(master_socket, &readfds))
        {
            int new_socket;
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                std::cerr << "Error on accept" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (client_socket.size() < MAX_CLIENTS)
            {
                static int client_id = 0;
                client_socket.push_back({new_socket, client_id++});
                std::cout << "New connection, client id: " << client_id - 1 << ", socket fd: " << new_socket << std::endl;
                static const std::string connection_accepted = "Connection accepted\n";
                send(new_socket, connection_accepted.c_str(), connection_accepted.size(), 0);
            }
            else
            {
                static const std::string connection_refused = "Connection refused\n";
                send(new_socket, connection_refused.c_str(), connection_refused.size(), 0);
                close(new_socket);
            }
        }

        for (int i = 0; i < client_socket.size(); ++i)
        {
            int sd = client_socket[i].first;
            if (FD_ISSET(sd, &readfds))
            {
                char buffer[BUF_SIZE + 1];
                int valread;
                if ((valread = read(sd, buffer, BUF_SIZE)) == 0)
                {
                    std::cout << "Client disconnected, id: " << client_socket[i].second << ", socket fd: " << sd << std::endl;

                    close(sd);
                    client_socket.erase(client_socket.begin() + i);
                }
                else
                {
                    buffer[valread] = '\0';
                    writeToFile(buffer, client_socket[i].second);
                }
            }
        }
    }
}