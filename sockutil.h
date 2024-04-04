#ifndef SOCKUTIL_SOCKUTIL_H
#define SOCKUTIL_SOCKUTIL_H

#include <cstring>
#include <iostream>
#include <string.h>
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
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#define NUM 6
#define MAX_LEN 512

int createTCPipv4socket();
struct CLIENT_INFO {
  int id;
  std::string name;
  int socket;
  std::thread th;
};
std::string color(int code);
void SET_NAME(int id, char name[]);

#endif // 
