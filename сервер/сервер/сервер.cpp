#include <iostream>
#include <fstream>
#include <string>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")


bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream stream1(file1, std::ios::binary);
    std::ifstream stream2(file2, std::ios::binary);

    if (!stream1.is_open() || !stream2.is_open()) {
        return false;
    }

    return std::istreambuf_iterator<char>(stream1) == std::istreambuf_iterator<char>() &&
        std::istreambuf_iterator<char>(stream2) == std::istreambuf_iterator<char>();
}
std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }



    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';

        std::ofstream outFile("received.txt");
        outFile.write(buffer, bytesReceived);
        outFile.close();
        std::string serverContent = readFile("server.txt");
        std::string clientContent = readFile("received.txt");


        if (serverContent == clientContent){
            const char* message = "equal";
            send(clientSocket, message, strlen(message), 0);
        }
        else {
            const char* message = "dif version";
            send(clientSocket, message, strlen(message), 0);
            std::ifstream exeFile("server.exe", std::ios::binary | std::ios::ate);
            if (exeFile.is_open()) {
                std::streamsize size = exeFile.tellg();
                exeFile.seekg(0, std::ios::beg);

                char* buffer = new char[size];
                exeFile.read(buffer, size);
                exeFile.close();

                send(clientSocket, buffer, size, 0);

                delete[] buffer;
            }
        }
        remove("received.txt");
    }
    else {
        std::cerr << "Failed to receive data" << std::endl;
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
