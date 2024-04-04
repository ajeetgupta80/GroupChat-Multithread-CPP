#include "sockutil.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#define NUM 6
#define MAX_LEN 512

int createTCPipv4socket() {
     return socket(AF_INET, SOCK_STREAM, 0); 
    }
