#include "pch.h"
#include <WinSock2.h>
#include <iostream>
#include <process.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

typedef struct data{
    SOCKET socket;
    std::string fileName;
} clientData;



unsigned int __stdcall clientSession(void *data) {
    clientData *client = (clientData*)data;
    //TODO clientSession body

    send(client->socket, client->fileName.c_str(), client->fileName.length(), 0);
    char buffer[4096];
	FILE *file = fopen(client->fileName.c_str(), "wb");
	if (file != nullptr) {
		long received;
		bool started = false;
		while ((received = recv(client->socket, buffer, 4096, 0)) > 0) {
			fwrite(buffer, 1, received, file);
			if (!started) {
				std::cout << "Receiving..." << std::endl;
				started = true;
			}
		}
		fclose(file);
		if (started)
			std::cout << "File received" << std::endl;
		else
			std::cout << "Error receiving the file" << std::endl;
	}
    closesocket(client->socket);
    delete client;
    return 0;
}


int main()
{
    WSADATA wsas;
    int error;
    WORD ver;
    ver = MAKEWORD(1, 1);
    error = WSAStartup(ver, &wsas);
    if (error != 0) {
        WSACleanup();
        return -1;
    }
    SOCKET server;
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        WSACleanup();
        return -2;
    }

    struct sockaddr_in sa;
    memset((void *)(&sa), 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(8080);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    error = bind(server, (struct sockaddr FAR*)&sa, sizeof(sa));
    if (error == SOCKET_ERROR) {
        WSACleanup();
        return -3;	
    }
    listen(server, 5);
    std::cout << "Server listening..." << std::endl;

    while (true) {
        SOCKET client;
        struct sockaddr_in sockaddr;
        int size;
        size = sizeof(sockaddr);
        std::string fileName;
        std::cout << "Waiting for connections..." << std::endl;
        client = accept(server, (struct sockaddr FAR*)&sockaddr, &size);
        std::cout << "Client connected" << std::endl;
        std::cout << "Type in file name to search on client's computer" << std::endl;
        std::cin >> fileName;
        clientData *data = new clientData;
        data->socket = client;
        data->fileName = fileName;
        unsigned int threadID;
        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &clientSession, (void*)data, 0, &threadID);

    }
    closesocket(server);
    WSACleanup();
    return 0;
}

