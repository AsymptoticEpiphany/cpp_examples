#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include "mpmc_queue.h"

using json = nlohmann::json;

// --------------------------
// Shared MPMC Queue
// --------------------------
constexpr size_t QUEUE_CAPACITY = 16384;
MPMCQueue<json, QUEUE_CAPACITY> tradeQueue;

// --------------------------
// TCP Reader Thread (Producer)
// --------------------------
void tcpReader(const std::string& host, int port, int producerId) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sock);
        return;
    }

    std::cout << "[Producer " << producerId << "] Connected to TRACE feed on port " << port << "\n";

    std::string buffer;
    char readBuf[1024];
    while (true) {
        ssize_t n = read(sock, readBuf, sizeof(readBuf));
        if (n <= 0) break;

        buffer.append(readBuf, n);

        size_t pos;
        while ((pos = buffer.find('\n')) != std::string::npos) {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);

            try {
                auto msg = json::parse(line);

                // Enqueue into MPMC queue (spin if full)
                while (!tradeQueue.enqueue(msg)) {
                    std::this_thread::sleep_for(std::chrono::microseconds(50));
                }

            } catch (json::parse_error& e) {
                std::cerr << "[Producer " << producerId << "] JSON parse error: " << e.what() << "\n";
            }
        }
    }

    close(sock);
    std::cout << "[Producer " << producerId << "] TCP connection closed\n";
}

// --------------------------
// Consumer Thread
// --------------------------
void consumer(int consumerId) {
    while (true) {
        json msg;
        while (!tradeQueue.dequeue(msg)) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        std::cout << "[Consumer " << consumerId << "] Got trade: " << msg.dump() << "\n";
    }
}

// --------------------------
// Main Function
// --------------------------
int main() {
    const std::string host = "127.0.0.1";
    const std::vector<int> ports = {5555, 5556, 5557}; // Each port runs a separate Python feed
    const int numConsumers = 2;

    // Launch producers (one per port)
    std::vector<std::thread> producers;
    for (size_t i = 0; i < ports.size(); ++i) {
        producers.emplace_back(tcpReader, host, ports[i], static_cast<int>(i + 1));
    }

    // Launch consumers
    std::vector<std::thread> consumers;
    for (int i = 0; i < numConsumers; ++i) {
        consumers.emplace_back(consumer, i + 1);
    }

    // Join threads (program runs indefinitely)
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    return 0;
}
