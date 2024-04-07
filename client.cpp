#include "sockutil.h"
#include <arpa/inet.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <errno.h>
#include <ios>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define NUM 6
#define MAX_LEN 512
#define CLIENT_SIZE sizeof(clients)

uint16_t PORT_NO;
int CLIENT_SOCKET;
bool EXIT_FLAG;
std::thread t_send;
std::thread t_recv;
std::string def_col = "\033[0m";
std::vector<std::string> colors = {"\033[31m", "\033[32m", "\033[33m",
                                   "\033[34m", "\033[35m", "\033[36m"};

void SEND_MESSAGE(int CLIENT_SOCKET);
void RECV_MESSAGE(int CLIENT_SOCKET);
std::string COLOR(int RANDOM);
void Erase_Text(int cnt);
void CATCH_CTRL_C(int signal);

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "argument in wrong order" << std::endl;
    return 0;
  }

  PORT_NO = atoi(argv[1]);

  CLIENT_SOCKET = socket(AF_INET, SOCK_STREAM, 0);
  if (CLIENT_SOCKET < 0) {
    perror("socket error :");
    exit(-1);
  }

  struct sockaddr_in client_addr;
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(PORT_NO);
  client_addr.sin_addr.s_addr = INADDR_ANY;

  bzero(&client_addr.sin_zero, 0);

  if (connect(CLIENT_SOCKET, (struct sockaddr *)&client_addr,
              sizeof(client_addr)) < 0) {
    perror("connect error :");
    exit(-1);
  }
  // std::cout << "connected to the server " << std::endl;
  signal(SIGINT, CATCH_CTRL_C);
  char name[MAX_LEN];
  std::cout << "Enter your name : ";
  std::cin.getline(name, MAX_LEN);
  send(CLIENT_SOCKET, name, sizeof(name), 0);

  std::cout << colors[NUM - 1] << "\n\t  ====== Welcome to the chat-room "
            << name << " =======" << std::endl
            << def_col;

  std::thread t1(SEND_MESSAGE, CLIENT_SOCKET);
  std::thread t2(RECV_MESSAGE, CLIENT_SOCKET);

  t_send = std::move(t1);
  t_recv = std::move(t2);

  if (t_send.joinable()) {
    t_send.join();
  }
  if (t_recv.joinable()) {
    t_recv.join();
  }

  return 0;
}

void SEND_MESSAGE(int CLIENT_SOCKET) {
  while (true) {
    std::cout << colors[1] << "You : " << def_col;
    char msg[MAX_LEN];
    std::cin.getline(msg, MAX_LEN);
    send(CLIENT_SOCKET, msg, sizeof(msg), 0);
    if (strcmp(msg, "#exit") == 0) {
      EXIT_FLAG = true;
      t_recv.detach();
      close(CLIENT_SOCKET);
      return;
    } else if (strcmp(msg, "#cli") == 0) {
      std::cout << "enter client name: ";

      char target[MAX_LEN];
      std::cin.getline(target, MAX_LEN);
      send(CLIENT_SOCKET, "#cli", sizeof("#cli"), 0);
      send(CLIENT_SOCKET, target, sizeof(target), 0);
    }
  }
}

void RECV_MESSAGE(int CLIENT_SOCKET) {
  while (true) {
    if (EXIT_FLAG) {
      return;
    }
    char name[MAX_LEN];
    char msg[MAX_LEN];
    int rand;
    int bytes_recv = recv(CLIENT_SOCKET, name, sizeof(name), 0);
    if (bytes_recv <= 0)
      continue;
    recv(CLIENT_SOCKET, &rand, sizeof(rand), 0);
    recv(CLIENT_SOCKET, msg, sizeof(msg), 0);
    Erase_Text(6);
    if (strcmp(name, "NEW_CON") != 0) {
      std::cout << COLOR(rand) << name << " : " << def_col << msg << std::endl;
    } else {
      std::cout << COLOR(rand) << msg << std::endl;
    }
    std::cout << colors[1] << "You : " << def_col;
    fflush(stdout);
  }
}

std::string COLOR(int RANDOM) { return colors[RANDOM % NUM]; }

void Erase_Text(int cnt) {
  char back_space = 8;
  for (int i = 0; i < cnt; ++i) {
    std::cout << back_space;
  }
}
void CATCH_CTRL_C(int signal) {
  char terminate[MAX_LEN] = "#exit";
  send(CLIENT_SOCKET, terminate, sizeof(terminate), 0);
  EXIT_FLAG = true;
  t_send.detach();
  t_recv.detach();
  close(CLIENT_SOCKET);
  exit(signal);
}
