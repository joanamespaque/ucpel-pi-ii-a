CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -std=c99 -O2
CFLAGS  += -Iinclude
TARGET  := network_monitor

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Windows builds (native MinGW or cross-compilation) need the IP Helper and Winsock libraries.
ifneq (,$(or $(findstring mingw,$(CC)),$(filter Windows_NT,$(OS))))
    LDLIBS += -liphlpapi -lws2_32
    EXT    := .exe
else
    CFLAGS += -D_DEFAULT_SOURCE
endif

BIN := $(BIN_DIR)/$(TARGET)$(EXT)

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
