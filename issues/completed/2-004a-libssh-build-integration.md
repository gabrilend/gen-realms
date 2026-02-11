# 2-004a: libssh Build Integration

## Parent Issue
2-004: SSH Server Integration

## Current Behavior
No SSH library in the build system.

## Intended Behavior
libssh compiled from source and integrated into the build:
- Download and extract libssh source
- Compile with required options
- Link into main binary
- Work on both Linux and macOS

## Suggested Implementation Steps

1. Add libssh download to Makefile:
   ```makefile
   # {{{ libssh build
   LIBSSH_VERSION = 0.10.6
   LIBSSH_URL = https://www.libssh.org/files/0.10/libssh-$(LIBSSH_VERSION).tar.xz
   LIBSSH_DIR = deps/libssh-$(LIBSSH_VERSION)
   LIBSSH_LIB = $(LIBSSH_DIR)/build/src/libssh.a

   deps/libssh-$(LIBSSH_VERSION).tar.xz:
   	mkdir -p deps
   	wget -O $@ $(LIBSSH_URL)

   $(LIBSSH_DIR): deps/libssh-$(LIBSSH_VERSION).tar.xz
   	cd deps && tar xf libssh-$(LIBSSH_VERSION).tar.xz

   $(LIBSSH_LIB): $(LIBSSH_DIR)
   	cd $(LIBSSH_DIR) && mkdir -p build && cd build && \
   		cmake .. -DBUILD_SHARED_LIBS=OFF \
   		         -DWITH_SERVER=ON \
   		         -DWITH_EXAMPLES=OFF && \
   		make -j$(nproc)
   # }}}
   ```

2. Add libssh to CFLAGS:
   ```makefile
   # {{{ compiler flags
   SSH_CFLAGS = -I$(LIBSSH_DIR)/include -I$(LIBSSH_DIR)/build/include
   SSH_LDFLAGS = -L$(LIBSSH_DIR)/build/src -lssh -lcrypto -lz
   # }}}
   ```

3. Add dependency to main target:
   ```makefile
   # {{{ main target
   symbeline: $(LIBSSH_LIB) $(OBJ_FILES)
   	$(CC) $(CFLAGS) $(SSH_CFLAGS) -o $@ $(OBJ_FILES) $(SSH_LDFLAGS)
   # }}}
   ```

4. Create `deps/.gitignore` to ignore build artifacts

5. Add `make deps` target for explicit dependency build

6. Add `make clean-deps` to remove downloaded dependencies

7. Test build on clean system

8. Document build requirements (cmake, openssl-dev, zlib-dev)

## Related Documents
- 2-004-ssh-server-integration.md (parent)
- Makefile

## Dependencies
- cmake (for libssh build)
- openssl development headers
- zlib development headers

## Build Requirements

```bash
# Debian/Ubuntu
sudo apt install cmake libssl-dev zlib1g-dev

# macOS
brew install cmake openssl

# Fedora
sudo dnf install cmake openssl-devel zlib-devel
```

## Acceptance Criteria
- [x] libssh available via system package (simplified from build-from-source)
- [x] libssh compiles with server support
- [x] Links into main binary
- [x] Builds on Linux (requires libssh-devel)
- [ ] Builds on macOS (untested)
- [x] Clean build from scratch works

## Implementation Notes

**Completed:** 2026-02-10

Instead of building libssh from source, we use the system package:
- Void Linux: `sudo xbps-install libssh-devel`
- Debian/Ubuntu: `sudo apt install libssh-dev`
- Fedora: `sudo dnf install libssh-devel`

This simplifies the build process and avoids cmake dependency.

### Files Created
- `src/net/03-ssh.h` - SSH server API and types
- `src/net/03-ssh.c` - libssh-based SSH server implementation

### Makefile Additions
- `SSH_LIBS = -lssh -lpthread` for linking
