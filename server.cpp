#include "sockutil.h"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <errno.h>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define NUM 6
#define MAX_LEN 512
#define CLIENT_SIZE sizeof(clients)

int u_id = 0;
std::mutex client_mtx;
std::mutex cout_mtx;
std::string def_col = "\033[0m";
std::vector<std::string> colors = {"\033[31m", "\033[32m", "\033[33m",
                                   "\033[34m", "\033[35m", "\033[36m"};
std::vector<CLIENT_INFO> clients;

void HANDLE_CLIENT(int client_socket, int id);
void BROADCASTING(std::string message, int SENDER_ID);
void END_CON(int ID);
void Shared_Print(std::string str, bool endline);
void BROADCAST_ID_FOR_COLOR(int num, int SENDER_ID);
int GET_CLIENT_COUNT();
std::string GetClientsInfo();

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "argument in wrong order" << std::endl;
    return 0;
  }

  int PORT_NO = atoi(argv[1]);

  int server_socket = createTCPipv4socket();
  if (server_socket < 0) {
    std::cout << "failed to create socket" << std::endl;
    exit(-1);
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT_NO);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  bzero(&server_addr.sin_zero, 0);

  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    std::cout << "failed bind" << std::endl;
    perror("bind error: ");
    exit(-1);
  }

  if (listen(server_socket, 8) < 0) {
    perror("listen error: ");
    exit(-1);
  }

  // std::cout << colors[NUM - 1]
  //          << "\n\t ===== server is listening... =====" << def_col
  //           << std::endl;
  std::cout << colors[NUM - 1]
            << "\n\t  ▄████▄   ██░ ██  ▄▄▄     ▄▄▄█████▓     ██████ ▓█████  "
               "██▀███   ██▒   █▓▓█████  ██▀███  \n"
               "\t▒██▀ ▀█  ▓██░ ██▒▒████▄   ▓  ██▒ ▓▒   ▒██    ▒ ▓█   ▀ ▓██ ▒ "
               "██▒▓██░   █▒▓█   ▀ ▓██ ▒ ██▒\n"
               "\t▒▓█    ▄ ▒██▀▀██░▒██  ▀█▄ ▒ ▓██░ ▒░   ░ ▓██▄   ▒███   ▓██ "
               "░▄█ ▒ ▓██  █▒░▒███   ▓██ ░▄█ ▒\n"
               "\t▒▓▓▄ ▄██▒░▓█ ░██ ░██▄▄▄▄██░ ▓██▓ ░      ▒   ██▒▒▓█  ▄ "
               "▒██▀▀█▄    ▒██ █░░▒▓█  ▄ ▒██▀▀█▄  \n"
               "\t▒ ▓███▀ ░░▓█▒░██▓ ▓█   ▓██▒ ▒██▒ ░    ▒██████▒▒░▒████▒░██▓ "
               "▒██▒   ▒▀█░  ░▒████▒░██▓ ▒██▒\n"
               "\t░ ░▒ ▒  ░ ▒ ░░▒░▒ ▒▒   ▓▒█░ ▒ ░░      ▒ ▒▓▒ ▒ ░░░ ▒░ ░░ ▒▓ "
               "░▒▓░   ░ ▐░  ░░ ▒░ ░░ ▒▓ ░▒▓░\n"
               "\t  ░  ▒    ▒ ░▒░ ░  ▒   ▒▒ ░   ░       ░ ░▒  ░ ░ ░ ░  ░\n";

  struct sockaddr_in client_addr;
  int client_socket;

  unsigned int len = sizeof(sockaddr_in);

  while (1) {
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
                                &len)) < 0) {
      perror("accept error:");
      exit(-1);
    }
    u_id++;
    std::thread t(HANDLE_CLIENT, client_socket, u_id);
    std::lock_guard<std::mutex> guard(client_mtx);
    std::string random = "anonymous";
    clients.push_back({u_id, random, client_socket, (move(t))});
  }

  for (int i = 0; i < clients.size(); i++) {
    if (clients[i].th.joinable()) {
      clients[i].th.join();
    }
  }
  close(server_socket);
  return 0;
}

void SET_NAME(int id, char name[]) {
  for (int i = 0; i < CLIENT_SIZE; ++i) {
    if (clients[i].id == id) {
      clients[i].name = std::string(name);
    }
  }
}

int GET_CLIENT_COUNT() { return clients.size(); }

void HANDLE_CLIENT(int client_socket, int id) {
  char name[MAX_LEN];
  char msg[MAX_LEN];
  recv(client_socket, name, sizeof(name), 0);
  SET_NAME(id, name);

  // welcome bitch
  std::string WELCOME =
      std::string(name) + std::string(" has joined the party");
  // std::cout << colors[NUM - 3] << WELCOME << std::endl;
  BROADCASTING("NEW_CON", id);
  BROADCAST_ID_FOR_COLOR(id, id);
  BROADCASTING(WELCOME, id);
  Shared_Print(color(id) + WELCOME + def_col, true);

  while (true) {
    int bytes_recv = recv(client_socket, msg, sizeof(msg), 0);
    if (bytes_recv <= 0) {
      return;
    }
    if (strcmp(msg, "#exit") == 0) {

      // leaving message
      std::string bye = std::string(name) + std::string(" has left");
      BROADCASTING("NEW_CON", id);
      BROADCAST_ID_FOR_COLOR(id, id);
      BROADCASTING(bye, id);
      Shared_Print(color(id) + bye + def_col, true);
      END_CON(id);
      return;
    }

    if (strcmp(msg, "#gc") == 0) {
      uint16_t COUNT = GET_CLIENT_COUNT();
      std::string INFO = GetClientsInfo();
      //  "number of active members ->: " + std::to_string(COUNT);
      char buff[MAX_LEN] = "NEW_CON";

      send(client_socket, buff, sizeof(buff), 0);
      send(client_socket, &id, sizeof(id), 0);
      send(client_socket, INFO.c_str(), INFO.length(), 0);
      continue;
    }
    if (strcmp(msg, "#cli") == 0) {
      std::cout << " cli function hitted" << std::endl;
      char target[MAX_LEN];
      recv(client_socket, target, sizeof(target), 0);

      int target_socket = -1;
      for (const auto &client : clients) {
        if (client.name == std::string(target)) {
          target_socket = client.socket;
          break;
        }
      }

      if (target_socket != -1) {
        std::string baat =
            "You are now chatting separately with " + std::string(name);
        // send(target_socket, baat.c_str(), sizeof(baat), 0);
        char buff[MAX_LEN] = "NEW_CON";

        send(target_socket, buff, sizeof(buff), 0);
        send(target_socket, &id, sizeof(id), 0);
        send(target_socket, baat.c_str(), baat.length(), 0);
        std::string bat =
            "You are now chatting separately with " + std::string(target);
        // send(client_socket, bat.c_str(), sizeof(bat), 0);
      } else {
        std::string tar_msg = "target not found";
        send(client_socket, tar_msg.c_str(), sizeof(tar_msg), 0);
      }
    }

    BROADCASTING(std::string(name), id);
    BROADCAST_ID_FOR_COLOR(id, id);
    BROADCASTING(std::string(msg), id);
    Shared_Print(color(id) + std::string(name) + " : " + def_col +
                     std::string(msg),
                 true);
  }
}

void BROADCAST_ID_FOR_COLOR(int num, int SENDER_ID) {
  for (int i = 0; i < CLIENT_SIZE; ++i) {
    if (clients[i].id != SENDER_ID) {
      send(clients[i].socket, &num, sizeof(num), 0);
    }
  }
}
void BROADCASTING(std::string message, int SENDER_ID) {

  char msg[MAX_LEN];
  strcpy(msg, message.c_str());
  for (int i = 0; i < CLIENT_SIZE; ++i) {
    if (clients[i].id != SENDER_ID) {
      send(clients[i].socket, msg, sizeof(msg), 0);
    }
  }
}

void END_CON(int ID) {
  for (int i = 0; i < CLIENT_SIZE; ++i) {
    if (clients[i].id == ID) {
      std::lock_guard<std::mutex> guard(client_mtx);
      clients[i].th.detach();
      clients.erase(clients.begin() + i);
      close(clients[i].socket);
      break;
    }
  }
}

std::string GetClientsInfo() {
  std::string clientInfo = "Active Members:\n";

  for (const auto &client : clients) {
    clientInfo +=
        "ID: " + std::to_string(client.id) + ", Name: " + client.name + "\n";
  }

  return clientInfo;
}

void Shared_Print(std::string msg, bool endline = true) {
  std::lock_guard<std::mutex> guard(cout_mtx);
  std::cout << msg;
  if (endline)
    std::cout << std::endl;
}

std::string color(int code) { return colors[code % NUM]; }
