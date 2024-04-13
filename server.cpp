#include "sockutil.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <errno.h>
#include <iostream>
#include <list>
#include <mutex>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
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
void SAVE_TO_CACHE(int u_id, std::string msg);
std::string Get_old_message();

// cache implementation with previous 5 messages
class LRUCache {
public:
  struct Node {
    std::string key;
    std::string value;
    Node *prev;
    Node *next;
    Node(const std::string &k, const std::string &v) {
      key = k;
      value = v;
    }
  };

  std::unordered_map<std::string, Node *> cacheMap;
  std::list<Node *> cacheList;
  int capacity;

public:
  LRUCache(int cap) { capacity = cap; }

  void put(const std::string &key, const std::string &value) {

    if (cacheList.size() >= capacity) {
      Node *last = cacheList.back();
      cacheMap.erase(last->key);
      cacheList.pop_back();
      delete last;
    }
    Node *newNode = new Node(key, value);
    cacheList.push_front(newNode);
    cacheMap[key] = newNode;
  }

private:
  void update(Node *node) {
    cacheList.erase(find(cacheList.begin(), cacheList.end(), node));
    cacheList.push_front(node);
  }
};

// cache implementation with replacing key so only latest message from each
// client
class Cache {
public:
  struct Node {
    std::string key;
    std::string value;
    Node *prev;
    Node *next;
    Node() {
      key = "";
      value = "";
    }
    Node(const std::string &k, const std::string &v) {
      key = k;
      value = v;
    }
  };
  std::unordered_map<std::string, Node *> cacheMap;
  Node *head = new Node();
  Node *tail = new Node();
  int CAPACITY;

  Cache(int cap) {
    CAPACITY = cap;
    head->next = tail;
    tail->prev = head;
  }

  void ADD_Node(Node *newnode);
  void Del_Node(Node *newnode);
  void PUT_DATA(const std::string &key, const std::string &value);
};

void Cache::PUT_DATA(const std::string &key, const std::string &value) {
  if (cacheMap.find(key) != cacheMap.end()) {
    Node *exist = cacheMap[key];
    cacheMap.erase(key);
    Del_Node(exist);
  }
  if (cacheMap.size() == CAPACITY) {
    cacheMap.erase(tail->prev->key);
    Del_Node(tail->prev);
  }
  ADD_Node(new Node(key, value));
  cacheMap[key] = head->next;
}

void Cache::Del_Node(Node *delnode) {
  Node *nodeprev = delnode->prev;
  Node *nodenext = delnode->next;
  nodeprev->next = nodenext;
  nodenext->prev = nodeprev;
}

void Cache::ADD_Node(Node *newnode) {
  Node *temp = head->next;
  newnode->next = temp;
  newnode->prev = head;
  head->next = newnode;
  temp->prev = newnode;
}

Cache cache(5);
// LRUCache cache(5);

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
  std::cout << colors[NUM - 6]
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
  std::cout << std::endl;
  std::cout << std::endl;
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
  std::string WELCOME = std::string("                        ") +
                        std::string(name) +
                        std::string(" has joined the party");
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
      std::string bye = std::string("                        ") +
                        std::string(name) + std::string(" has left");
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
    if (strcmp(msg, "#getmsg") == 0) {

      std::string INFO_MSG = Get_old_message();

      //  "number of active members ->: " + std::to_string(COUNT);
      char buff[MAX_LEN] = "NEW_CONN";
      size_t INFO_SIZE = INFO_MSG.size();
      send(client_socket, buff, sizeof(buff), 0);
      send(client_socket, &id, sizeof(id), 0);
      send(client_socket, INFO_MSG.c_str(), INFO_SIZE, 0);
      continue;
    }

    BROADCASTING(std::string(name), id);
    BROADCAST_ID_FOR_COLOR(id, id);
    SAVE_TO_CACHE(id, msg);
    BROADCASTING(std::string(msg), id);
    Shared_Print(color(id) + std::string("                        ") +
                     std::string(name) + " : " + def_col + std::string(msg),
                 true);
  }
}

void SAVE_TO_CACHE(int id, std::string msg) {
  cache.PUT_DATA(std::to_string(id), msg);
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

std::string Get_old_message() {
  std::string oldmsg = "last 5 messages:\n";

  for (const auto &it : cache.cacheMap) {

    // Append the ID to the message
    oldmsg += "ID: " + it.first + " msg: " + it.second->value + "\n";
  }
  return oldmsg;
}

void Shared_Print(std::string msg, bool endline = true) {
  std::lock_guard<std::mutex> guard(cout_mtx);
  std::cout << msg;
  if (endline)
    std::cout << std::endl;
}

std::string color(int code) { return colors[code % NUM]; }
