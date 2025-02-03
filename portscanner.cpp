#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h> // inet_pton için gerekli başlık dosyası

#pragma comment(lib, "ws2_32.lib")

std::mutex mtx;
std::ofstream outputFile;

void scanPort(const std::string& ip, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // inet_addr yerine inet_pton kullanıyoruz
    if (inet_pton(AF_INET, ip.c_str(), &server.sin_addr) <= 0) {
        std::cerr << "Invalid IP address." << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0) {
        mtx.lock();
        std::cout << "Port " << port << " is open on " << ip << std::endl;
        outputFile << "Port " << port << " is open on " << ip << std::endl;
        mtx.unlock();
    }

    closesocket(sock);
    WSACleanup();
}

void scanIPRange(const std::string& ip, int startPort, int endPort) {
    std::vector<std::thread> threads;

    for (int port = startPort; port <= endPort; ++port) {
        threads.emplace_back(scanPort, ip, port);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

int main() {
    std::string ip;
    int startPort, endPort;

    std::cout << "Enter IP address: ";
    std::cin >> ip;
    std::cout << "Enter start port: ";
    std::cin >> startPort;
    std::cout << "Enter end port: ";
    std::cin >> endPort;

    outputFile.open("scan_results.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file." << std::endl;
        return 1;
    }

    std::cout << "Scanning ports " << startPort << " to " << endPort << " on " << ip << "..." << std::endl;
    scanIPRange(ip, startPort, endPort);

    outputFile.close();
    std::cout << "Scan completed. Results saved to scan_results.txt." << std::endl;

    return 0;
}