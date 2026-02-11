# 0-001d: Makefile Local Dependency Priority

## Parent Issue
0-001: Dependency Management System

## Current Behavior
Makefile assumes system-installed dependencies only.

## Intended Behavior
Makefile that:
- Checks for local dependencies first (`libs/local/`)
- Falls back to system dependencies if local not found
- Uses static linking for local deps (portability)
- Reports which dependency source is being used

## Suggested Implementation Steps

1. Add dependency detection to Makefile:
   ```makefile
   # {{{ dependency detection
   # Check for local dependencies first, fall back to system
   LOCAL_DEPS_PREFIX = libs/local

   # libwebsockets detection
   ifneq ($(wildcard $(LOCAL_DEPS_PREFIX)/lib/libwebsockets.a),)
       # Use local libwebsockets
       WEBSOCKET_CFLAGS = -I$(LOCAL_DEPS_PREFIX)/include
       WEBSOCKET_LIBS = $(LOCAL_DEPS_PREFIX)/lib/libwebsockets.a
       WEBSOCKET_SOURCE = local
   else ifneq ($(shell pkg-config --exists libwebsockets 2>/dev/null && echo yes),)
       # Use system libwebsockets via pkg-config
       WEBSOCKET_CFLAGS = $(shell pkg-config --cflags libwebsockets)
       WEBSOCKET_LIBS = $(shell pkg-config --libs libwebsockets)
       WEBSOCKET_SOURCE = system
   else
       # Fallback to simple -l flag
       WEBSOCKET_CFLAGS =
       WEBSOCKET_LIBS = -lwebsockets
       WEBSOCKET_SOURCE = system-fallback
   endif

   # libssh detection
   ifneq ($(wildcard $(LOCAL_DEPS_PREFIX)/lib/libssh.a),)
       # Use local libssh
       SSH_CFLAGS = -I$(LOCAL_DEPS_PREFIX)/include
       SSH_LIBS = $(LOCAL_DEPS_PREFIX)/lib/libssh.a -lcrypto -lpthread
       SSH_SOURCE = local
   else ifneq ($(shell pkg-config --exists libssh 2>/dev/null && echo yes),)
       # Use system libssh via pkg-config
       SSH_CFLAGS = $(shell pkg-config --cflags libssh)
       SSH_LIBS = $(shell pkg-config --libs libssh) -lpthread
       SSH_SOURCE = system
   else
       # Fallback
       SSH_CFLAGS =
       SSH_LIBS = -lssh -lpthread
       SSH_SOURCE = system-fallback
   endif
   # }}}
   ```

2. Update CFLAGS to include dependency flags:
   ```makefile
   CFLAGS = -Wall -Wextra -pedantic -std=c11 -g \
            $(WEBSOCKET_CFLAGS) $(SSH_CFLAGS)
   ```

3. Add deps-info target:
   ```makefile
   # {{{ deps-info
   deps-info:
   	@echo "Dependency sources:"
   	@echo "  libwebsockets: $(WEBSOCKET_SOURCE)"
   	@echo "  libssh:        $(SSH_SOURCE)"
   	@echo ""
   	@echo "libwebsockets:"
   	@echo "  CFLAGS: $(WEBSOCKET_CFLAGS)"
   	@echo "  LIBS:   $(WEBSOCKET_LIBS)"
   	@echo ""
   	@echo "libssh:"
   	@echo "  CFLAGS: $(SSH_CFLAGS)"
   	@echo "  LIBS:   $(SSH_LIBS)"
   # }}}
   ```

4. Add deps target to run installer:
   ```makefile
   # {{{ deps
   deps:
   	./scripts/install-deps.sh

   deps-force:
   	./scripts/install-deps.sh --force
   # }}}
   ```

5. Update clean to not remove deps:
   ```makefile
   clean:
   	rm -rf $(BUILD_DIR) $(BIN_DIR)

   clean-deps:
   	rm -rf libs/local libs/src
   ```

## Related Documents
- 0-001-dependency-management-system.md (parent)
- Makefile

## Acceptance Criteria
- [ ] Local deps detected when present
- [ ] Falls back to system deps gracefully
- [ ] Static linking used for local deps
- [ ] deps-info reports sources correctly
- [ ] deps target runs installer
- [ ] clean-deps removes local deps
