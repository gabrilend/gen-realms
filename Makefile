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

# cJSON library for JSON parsing (when available)
# CJSON_LIBS = -lcjson

# Directories
SRC_DIR = src
CLIENT_DIR = $(SRC_DIR)/client
CORE_DIR = $(SRC_DIR)/core
BUILD_DIR = build
BIN_DIR = bin

# Output targets
TERMINAL_BIN = $(BIN_DIR)/symbeline-terminal
TEST_TERMINAL_BIN = $(BIN_DIR)/test-terminal
# }}}

# {{{ source files
# Terminal client sources
TERMINAL_SOURCES = \
	$(CLIENT_DIR)/01-terminal.c

# Core game sources (Track A: 1-001, 1-002, 1-003)
CORE_SOURCES = \
	$(CORE_DIR)/01-card.c \
	$(CORE_DIR)/02-deck.c \
	$(CORE_DIR)/03-player.c

# Test sources
TEST_TERMINAL_SOURCES = \
	tests/test-terminal.c \
	$(TERMINAL_SOURCES)

TEST_CORE_SOURCES = \
	tests/test-core.c \
	$(CORE_SOURCES)
# }}}

# {{{ object files
TERMINAL_OBJECTS = $(TERMINAL_SOURCES:%.c=$(BUILD_DIR)/%.o)
CORE_OBJECTS = $(CORE_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_TERMINAL_OBJECTS = $(TEST_TERMINAL_SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_CORE_OBJECTS = $(TEST_CORE_SOURCES:%.c=$(BUILD_DIR)/%.o)
# }}}

# {{{ output binaries
TEST_CORE_BIN = $(BIN_DIR)/test-core
# }}}

# {{{ build targets
.PHONY: all clean terminal test test-core test-terminal dirs

all: dirs terminal

dirs:
	@mkdir -p $(BUILD_DIR)/$(CLIENT_DIR)
	@mkdir -p $(BUILD_DIR)/$(CORE_DIR)
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
