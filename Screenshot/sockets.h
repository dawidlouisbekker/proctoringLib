#pragma once
#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>


using std::unique_ptr;
// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")


struct socketInstance {
    SOCKET sock;

    socketInstance(SOCKET Sock) : sock(Sock) {}

    ~socketInstance() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
            std::cout << "Socket closed." << std::endl;
        }
    }
};

unique_ptr<socketInstance> vidSock;
unique_ptr<socketInstance> screenShotSock;

//Will join with main thread. Rest will only join main thread upon termination. 
unique_ptr<socketInstance> sessionSocket;

extern "C" int startSockets() {
    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serverAddr;
    const char* serverIP = "127.0.0.1";  // Replace with your server's IP
    int port = 8080;  // Replace with your server's port
    const char* message = "Hello, Server!";
    //First handle video socket
    
    // Step 1: Initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
        return 1;
    }

    // Step 2: Create a socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Step 3: Set up the server address structure
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    // Step 4: Connect to the server
    result = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Connection failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    //Send a to the server
    result = send(sock, message, (int)strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Message sent to server!" << std::endl;

    //Receive a response from the server
    char buffer[512];
    result = recv(sock, buffer, sizeof(buffer), 0);
    if (511 > result > 0) {
        buffer[result] = '\0';  // Null terminate the received data
        std::cout << "Received from server: " << buffer << std::endl;
    }
    else if (result == 0) {
        std::cout << "Connection closed by server." << std::endl;
    }
    else {
        std::cerr << "Recv failed with error: " << WSAGetLastError() << std::endl;
    }

    //Clean up and close the socket
    closesocket(sock);
    WSACleanup();

    return 0;
}

