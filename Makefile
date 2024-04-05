CXX = g++
CXXFLAGS = -std=c++11 -Wall
BINDIR = bin

SRCS_SERVER = server.cpp sockutil.cpp
SRCS_CLIENT = client.cpp sockutil.cpp
OBJS_SERVER = $(patsubst %.cpp,$(BINDIR)/%.o,$(SRCS_SERVER))
OBJS_CLIENT = $(patsubst %.cpp,$(BINDIR)/%.o,$(SRCS_CLIENT))
TARGET_SERVER = server
TARGET_CLIENT = client

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): $(OBJS_SERVER)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BINDIR)/%.o: %.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR) $(TARGET_SERVER) $(TARGET_CLIENT)

.PHONY: all clean
