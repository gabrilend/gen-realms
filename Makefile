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
CFLAGS_BASE = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS =

# ncurses library for terminal client
NCURSES_LIBS = -lncurses

# Math library for cJSON
MATH_LIBS = -lm
# }}}

# {{{ dependency detection
# Check for local dependencies first, fall back to system
LOCAL_DEPS_PREFIX = libs/local

# libwebsockets detection
ifneq ($(wildcard $(LOCAL_DEPS_PREFIX)/lib/libwebsockets.a),)
    # Use local libwebsockets (static)
    WEBSOCKET_CFLAGS = -I$(LOCAL_DEPS_PREFIX)/include
    WEBSOCKET_LIBS = $(LOCAL_DEPS_PREFIX)/lib/libwebsockets.a
    WEBSOCKET_SOURCE = local
else ifneq ($(shell pkg-config --exists libwebsockets 2>/dev/null && echo yes),)
    # Use system libwebsockets via pkg-config
    WEBSOCKET_CFLAGS = $(shell pkg-config --cflags libwebsockets)
    WEBSOCKET_LIBS = $(shell pkg-config --libs libwebsockets)
    WEBSOCKET_SOURCE = system-pkgconfig
else
    # Fallback to simple -l flag (may fail if not installed)
    WEBSOCKET_CFLAGS =
    WEBSOCKET_LIBS = -lwebsockets
    WEBSOCKET_SOURCE = system-fallback
endif

# libssh detection
ifneq ($(wildcard $(LOCAL_DEPS_PREFIX)/lib/libssh.a),)
    # Use local libssh (static, needs OpenSSL)
    SSH_CFLAGS = -I$(LOCAL_DEPS_PREFIX)/include
    SSH_LIBS = $(LOCAL_DEPS_PREFIX)/lib/libssh.a -lcrypto -lpthread
    SSH_SOURCE = local
else ifneq ($(shell pkg-config --exists libssh 2>/dev/null && echo yes),)
    # Use system libssh via pkg-config
    SSH_CFLAGS = $(shell pkg-config --cflags libssh)
    SSH_LIBS = $(shell pkg-config --libs libssh) -lpthread
    SSH_SOURCE = system-pkgconfig
else
    # Fallback to simple -l flag
    SSH_CFLAGS =
    SSH_LIBS = -lssh -lpthread
    SSH_SOURCE = system-fallback
endif

# Combine flags
CFLAGS = $(CFLAGS_BASE) $(WEBSOCKET_CFLAGS) $(SSH_CFLAGS)

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
DEMO_BIN = $(BIN_DIR)/phase-1-demo
DEMO2_BIN = $(BIN_DIR)/phase-2-demo
TEST_TERMINAL_BIN = $(BIN_DIR)/test-terminal
TEST_CONFIG_BIN = $(BIN_DIR)/test-config
TEST_HTTP_BIN = $(BIN_DIR)/test-http
TEST_SSH_BIN = $(BIN_DIR)/test-ssh
TEST_SERIALIZE_BIN = $(BIN_DIR)/test-serialize
TEST_PROTOCOL_BIN = $(BIN_DIR)/test-protocol
TEST_WEBSOCKET_BIN = $(BIN_DIR)/test-websocket
TEST_CONNECTIONS_BIN = $(BIN_DIR)/test-connections
TEST_SESSIONS_BIN = $(BIN_DIR)/test-sessions
TEST_HIDDEN_INFO_BIN = $(BIN_DIR)/test-hidden-info
TEST_VALIDATION_BIN = $(BIN_DIR)/test-validation
# }}}

# {{{ source files
# Terminal client sources
TERMINAL_SOURCES = \
	$(CLIENT_DIR)/01-terminal.c

# Core game sources (Track A: 1-001 through 1-008)
CORE_SOURCES = \
	$(CORE_DIR)/01-card.c \
	$(CORE_DIR)/02-deck.c \
	$(CORE_DIR)/03-player.c \
	$(CORE_DIR)/04-trade-row.c \
	$(CORE_DIR)/05-game.c \
	$(CORE_DIR)/06-combat.c \
	$(CORE_DIR)/07-effects.c \
	$(CORE_DIR)/08-auto-draw.c

# Network sources (Track B: 2-001, 2-002, 2-004)
NET_SOURCES = \
	$(NET_DIR)/01-config.c \
	$(NET_DIR)/02-http.c \
	$(NET_DIR)/03-ssh.c

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
	$(NET_DIR)/01-config.c \
	$(NET_DIR)/02-http.c \
	$(CJSON_SOURCES)

TEST_SSH_SOURCES = \
	tests/test-ssh.c \
	$(NET_DIR)/01-config.c \
	$(NET_DIR)/03-ssh.c \
	$(CJSON_SOURCES)

TEST_SERIALIZE_SOURCES = \
	tests/test-serialize.c \
	$(CORE_SOURCES) \
	$(CORE_DIR)/09-serialize.c \
	$(CJSON_SOURCES)

# Demo sources (Phase 1 demo: 1-013)
DEMO_DIR = $(SRC_DIR)/demo
DEMO_SOURCES = \
	$(DEMO_DIR)/phase-1-demo.c \
	$(CORE_SOURCES)

# Phase 2 demo sources (2-010)
# Includes stubs for ws_send/ssh_send since demo simulates networking
DEMO2_SOURCES = \
	$(DEMO_DIR)/phase-2-demo.c \
	$(DEMO_DIR)/phase-2-stubs.c \
	$(CORE_SOURCES) \
	$(CORE_DIR)/09-serialize.c \
	$(NET_DIR)/04-protocol.c \
	$(NET_DIR)/06-connections.c \
	$(NET_DIR)/07-sessions.c \
	$(NET_DIR)/08-validation.c \
	$(CJSON_SOURCES)

TEST_PROTOCOL_SOURCES = \
	tests/test-protocol.c \
	$(NET_DIR)/04-protocol.c \
	$(CORE_SOURCES) \
	$(CORE_DIR)/09-serialize.c \
	$(CJSON_SOURCES)

TEST_WEBSOCKET_SOURCES = \
	tests/test-websocket.c \
	$(NET_DIR)/05-websocket.c \
	$(NET_DIR)/04-protocol.c \
	$(NET_DIR)/02-http.c \
	$(NET_DIR)/01-config.c \
	$(CORE_SOURCES) \
	$(CORE_DIR)/09-serialize.c \
	$(CJSON_SOURCES)

# Connection manager tests (uses stubs for WS/SSH)
TEST_CONNECTIONS_SOURCES = \
	tests/test-connections.c \
	$(NET_DIR)/06-connections.c

# Session manager tests (needs core game module)
TEST_SESSIONS_SOURCES = \
	tests/test-sessions.c \
	$(NET_DIR)/07-sessions.c \
	$(CORE_SOURCES)

# Hidden information tests (needs serialization + core)
TEST_HIDDEN_INFO_SOURCES = \
	tests/test-hidden-info.c \
	$(CORE_SOURCES) \
	$(CORE_DIR)/09-serialize.c \
	$(CJSON_SOURCES)

# Validation tests (needs validation, protocol, core)
TEST_VALIDATION_SOURCES = \
	tests/test-validation.c \
	$(NET_DIR)/08-validation.c \
	$(CORE_SOURCES) \
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
TEST_SSH_OBJECTS = $(TEST_SSH_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_SERIALIZE_OBJECTS = $(TEST_SERIALIZE_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_PROTOCOL_OBJECTS = $(TEST_PROTOCOL_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_WEBSOCKET_OBJECTS = $(TEST_WEBSOCKET_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_CONNECTIONS_OBJECTS = $(TEST_CONNECTIONS_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_SESSIONS_OBJECTS = $(TEST_SESSIONS_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_HIDDEN_INFO_OBJECTS = $(TEST_HIDDEN_INFO_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_VALIDATION_OBJECTS = $(TEST_VALIDATION_SOURCES:%.c=$(BUILD_DIR)/%.o)
DEMO_OBJECTS = $(DEMO_SOURCES:%.c=$(BUILD_DIR)/%.o)
DEMO2_OBJECTS = $(DEMO2_SOURCES:%.c=$(BUILD_DIR)/%.o)
# }}}

# {{{ output binaries
TEST_CORE_BIN = $(BIN_DIR)/test-core
# }}}

# {{{ build targets
.PHONY: all clean terminal server demo demo2 test test-core test-terminal test-config test-http test-ssh test-serialize test-protocol test-websocket test-connections test-sessions test-hidden-info test-validation dirs deps deps-force deps-info clean-deps

all: dirs terminal

dirs:
	@mkdir -p $(BUILD_DIR)/$(CLIENT_DIR)
	@mkdir -p $(BUILD_DIR)/$(CORE_DIR)
	@mkdir -p $(BUILD_DIR)/$(NET_DIR)
	@mkdir -p $(BUILD_DIR)/$(LIBS_DIR)
	@mkdir -p $(BUILD_DIR)/$(DEMO_DIR)
	@mkdir -p $(BUILD_DIR)/tests
	@mkdir -p $(BIN_DIR)

# Terminal client
terminal: dirs $(TERMINAL_BIN)

$(TERMINAL_BIN): $(TERMINAL_OBJECTS) $(BUILD_DIR)/$(CLIENT_DIR)/terminal-main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(NCURSES_LIBS)

# Phase 1 Demo (1-013)
demo: dirs $(DEMO_BIN)
	@echo "Demo built. Run with: ./$(DEMO_BIN)"

$(DEMO_BIN): $(DEMO_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Phase 2 Demo (2-010)
demo2: dirs $(DEMO2_BIN)
	@echo "Phase 2 demo built. Run with: ./$(DEMO2_BIN)"

$(DEMO2_BIN): $(DEMO2_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(MATH_LIBS)

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

# SSH tests (Track B: 2-004, requires libssh)
test-ssh: dirs $(TEST_SSH_BIN)
	./$(TEST_SSH_BIN)

$(TEST_SSH_BIN): $(TEST_SSH_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(SSH_LIBS) $(MATH_LIBS)

# Serialize tests (Track A: 1-012)
test-serialize: dirs $(TEST_SERIALIZE_BIN)
	./$(TEST_SERIALIZE_BIN)

$(TEST_SERIALIZE_BIN): $(TEST_SERIALIZE_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(MATH_LIBS)

# Protocol tests (Track B: 2-005)
test-protocol: dirs $(TEST_PROTOCOL_BIN)
	./$(TEST_PROTOCOL_BIN)

$(TEST_PROTOCOL_BIN): $(TEST_PROTOCOL_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(MATH_LIBS)

# WebSocket tests (Track B: 2-003, requires libwebsockets)
test-websocket: dirs $(TEST_WEBSOCKET_BIN)
	./$(TEST_WEBSOCKET_BIN)

$(TEST_WEBSOCKET_BIN): $(TEST_WEBSOCKET_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(WEBSOCKET_LIBS) $(MATH_LIBS)

# Connection manager tests (Track B: 2-006)
test-connections: dirs $(TEST_CONNECTIONS_BIN)
	./$(TEST_CONNECTIONS_BIN)

$(TEST_CONNECTIONS_BIN): $(TEST_CONNECTIONS_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Session manager tests (Track B: 2-007)
test-sessions: dirs $(TEST_SESSIONS_BIN)
	./$(TEST_SESSIONS_BIN)

$(TEST_SESSIONS_BIN): $(TEST_SESSIONS_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Hidden information tests (Track B: 2-008)
test-hidden-info: dirs $(TEST_HIDDEN_INFO_BIN)
	./$(TEST_HIDDEN_INFO_BIN)

$(TEST_HIDDEN_INFO_BIN): $(TEST_HIDDEN_INFO_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(MATH_LIBS)

# Input validation tests (Track B: 2-009)
test-validation: dirs $(TEST_VALIDATION_BIN)
	./$(TEST_VALIDATION_BIN)

$(TEST_VALIDATION_BIN): $(TEST_VALIDATION_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(MATH_LIBS)

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

# {{{ dependency targets
# Install all dependencies from source
deps:
	./scripts/install-deps.sh

# Force reinstall all dependencies
deps-force:
	./scripts/install-deps.sh --force

# Show detected dependency sources
deps-info:
	@echo "Dependency Detection"
	@echo "===================="
	@echo ""
	@echo "libwebsockets:"
	@echo "  Source:  $(WEBSOCKET_SOURCE)"
	@echo "  CFLAGS:  $(WEBSOCKET_CFLAGS)"
	@echo "  LIBS:    $(WEBSOCKET_LIBS)"
	@echo ""
	@echo "libssh:"
	@echo "  Source:  $(SSH_SOURCE)"
	@echo "  CFLAGS:  $(SSH_CFLAGS)"
	@echo "  LIBS:    $(SSH_LIBS)"
	@echo ""
	@echo "To install local dependencies:"
	@echo "  make deps"

# Remove locally installed dependencies
clean-deps:
	rm -rf libs/local libs/src
	@echo "Local dependencies removed"
# }}}
