# Makefile - Symbeline Realms native build configuration
#
# Builds the terminal client and server components using standard C tooling.
# For WebAssembly builds, use Makefile.wasm instead.
#
# Usage:
#   make            - Build all targets
#   make terminal   - Build terminal client only
#   make clean      - Remove build artifacts
#   make test       - Build and run tests

# {{{ configuration
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS =

# ncurses library for terminal client
NCURSES_LIBS = -lncurses

# libwebsockets for HTTP/WebSocket server
# Install: xbps-install libwebsockets-devel
WEBSOCKET_LIBS = -lwebsockets

# Math library for cJSON
MATH_LIBS = -lm

# Directories
SRC_DIR = src
CLIENT_DIR = $(SRC_DIR)/client
CORE_DIR = $(SRC_DIR)/core
NET_DIR = $(SRC_DIR)/net
LIBS_DIR = libs
BUILD_DIR = build
BIN_DIR = bin

# Output targets
TERMINAL_BIN = $(BIN_DIR)/symbeline-terminal
SERVER_BIN = $(BIN_DIR)/symbeline-server
TEST_TERMINAL_BIN = $(BIN_DIR)/test-terminal
TEST_CONFIG_BIN = $(BIN_DIR)/test-config
TEST_HTTP_BIN = $(BIN_DIR)/test-http
# }}}

# {{{ source files
# Terminal client sources
TERMINAL_SOURCES = \
	$(CLIENT_DIR)/01-terminal.c

# Core game sources (Track A: 1-001 through 1-007)
CORE_SOURCES = \
	$(CORE_DIR)/01-card.c \
	$(CORE_DIR)/02-deck.c \
	$(CORE_DIR)/03-player.c \
	$(CORE_DIR)/04-trade-row.c \
	$(CORE_DIR)/05-game.c \
	$(CORE_DIR)/06-combat.c \
	$(CORE_DIR)/07-effects.c

# Network sources (Track B: 2-001, 2-002)
NET_SOURCES = \
	$(NET_DIR)/01-config.c \
	$(NET_DIR)/02-http.c

# cJSON library source
CJSON_SOURCES = \
	$(LIBS_DIR)/cJSON.c

# Test sources
TEST_TERMINAL_SOURCES = \
	tests/test-terminal.c \
	$(TERMINAL_SOURCES)

TEST_CORE_SOURCES = \
	tests/test-core.c \
	$(CORE_SOURCES)

TEST_CONFIG_SOURCES = \
	tests/test-config.c \
	$(NET_DIR)/01-config.c \
	$(CJSON_SOURCES)

TEST_HTTP_SOURCES = \
	tests/test-http.c \
	$(NET_SOURCES) \
	$(CJSON_SOURCES)
# }}}

# {{{ object files
TERMINAL_OBJECTS = $(TERMINAL_SOURCES:%.c=$(BUILD_DIR)/%.o)
CORE_OBJECTS = $(CORE_SOURCES:%.c=$(BUILD_DIR)/%.o)
NET_OBJECTS = $(NET_SOURCES:%.c=$(BUILD_DIR)/%.o)
CJSON_OBJECTS = $(CJSON_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_TERMINAL_OBJECTS = $(TEST_TERMINAL_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_CORE_OBJECTS = $(TEST_CORE_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_CONFIG_OBJECTS = $(TEST_CONFIG_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_HTTP_OBJECTS = $(TEST_HTTP_SOURCES:%.c=$(BUILD_DIR)/%.o)
# }}}

# {{{ output binaries
TEST_CORE_BIN = $(BIN_DIR)/test-core
# }}}

# {{{ build targets
.PHONY: all clean terminal server test test-core test-terminal test-config dirs

all: dirs terminal

dirs:
	@mkdir -p $(BUILD_DIR)/$(CLIENT_DIR)
	@mkdir -p $(BUILD_DIR)/$(CORE_DIR)
	@mkdir -p $(BUILD_DIR)/$(NET_DIR)
	@mkdir -p $(BUILD_DIR)/$(LIBS_DIR)
	@mkdir -p $(BUILD_DIR)/tests
	@mkdir -p $(BIN_DIR)

# Terminal client
terminal: dirs $(TERMINAL_BIN)

$(TERMINAL_BIN): $(TERMINAL_OBJECTS) $(BUILD_DIR)/$(CLIENT_DIR)/terminal-main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(NCURSES_LIBS)

# Test programs - run core tests (main test target)
test: test-core

# Core tests (Track A: 1-001, 1-002, 1-003)
test-core: dirs $(TEST_CORE_BIN)
	./$(TEST_CORE_BIN)

$(TEST_CORE_BIN): $(TEST_CORE_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Terminal tests (requires ncurses)
test-terminal: dirs $(TEST_TERMINAL_BIN)
	./$(TEST_TERMINAL_BIN)

$(TEST_TERMINAL_BIN): $(TEST_TERMINAL_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(NCURSES_LIBS)

# Config tests (Track B: 2-001)
test-config: dirs $(TEST_CONFIG_BIN)
	./$(TEST_CONFIG_BIN)

$(TEST_CONFIG_BIN): $(TEST_CONFIG_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(MATH_LIBS)

# HTTP tests (Track B: 2-002, requires libwebsockets)
test-http: dirs $(TEST_HTTP_BIN)
	./$(TEST_HTTP_BIN)

$(TEST_HTTP_BIN): $(TEST_HTTP_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(WEBSOCKET_LIBS) $(MATH_LIBS)

# Object file compilation
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
# }}}

# {{{ development targets
# Compile with debug symbols and no optimization
debug: CFLAGS += -O0 -DDEBUG
debug: all

# Compile with optimizations
release: CFLAGS = -Wall -Wextra -O2 -DNDEBUG
release: all

# Check for memory leaks (requires valgrind)
memcheck: test
	valgrind --leak-check=full ./$(TEST_TERMINAL_BIN)
# }}}
