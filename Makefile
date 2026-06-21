CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -O2 -g

CLIENT_TARGET := client.exe
SERVER_TARGET := server.exe
BUILD_DIR := build

CLIENT_SRCS := $(wildcard client/*.c)
SERVER_SRCS := $(wildcard server/src/*.c)
CLIENT_OBJS := $(patsubst client/%.c,$(BUILD_DIR)/client_%.o,$(CLIENT_SRCS))
SERVER_OBJS := $(patsubst server/src/%.c,$(BUILD_DIR)/server_%.o,$(SERVER_SRCS))

INCLUDES := -Icommon/include -Iclient -Iserver/include
NCURSES_CFLAGS := $(shell pkg-config --cflags ncursesw 2>/dev/null || pkg-config --cflags ncurses 2>/dev/null)
NCURSES_LIBS := $(shell pkg-config --libs ncursesw 2>/dev/null || pkg-config --libs ncurses 2>/dev/null || echo -lncurses)

.PHONY: all client server clean

all: server client

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

server: $(SERVER_TARGET)

client: $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $@

$(CLIENT_TARGET): $(CLIENT_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o $@ $(NCURSES_LIBS)

$(BUILD_DIR)/client_%.o: client/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(NCURSES_CFLAGS) -c $< -o $@

$(BUILD_DIR)/server_%.o: server/src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(CLIENT_TARGET) $(SERVER_TARGET) $(TEST_TARGET)
