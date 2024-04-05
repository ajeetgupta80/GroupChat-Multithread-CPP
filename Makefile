CC = g++
CFLAGS = -Wall -Wextra -std=c++11

# Define the source files for the server and client
SERVER_SRC = server.cpp socketutil.cpp
CLIENT_SRC = client.cpp socketutil.cpp

# Define the object files for the server and client
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)

# Define the target executable names
SERVER_EXE = server.exe
CLIENT_EXE = client.exe

all: $(SERVER_EXE) $(CLIENT_EXE)

# Compile server source files
$(SERVER_OBJ): %.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Compile client source files
$(CLIENT_OBJ): %.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link server object files into the server executable
$(SERVER_EXE): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Link client object files into the client executable
$(CLIENT_EXE): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_EXE) $(CLIENT_EXE)

.PHONY: all clean
