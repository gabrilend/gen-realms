# 0-001a: libwebsockets Install Script

## Parent Issue
0-001: Dependency Management System

## Current Behavior
libwebsockets must be installed via system package manager.

## Intended Behavior
Shell script that:
- Downloads libwebsockets source from GitHub
- Compiles with CMake (minimal build, static library)
- Installs to local prefix
- Checks for updates before rebuilding
- Supports --force and --prefix flags

## Suggested Implementation Steps

1. Create `scripts/deps/install-libwebsockets.sh`:
   ```bash
   #!/bin/bash
   # {{{ install-libwebsockets.sh
   # Downloads and compiles libwebsockets from source.
   # Usage: ./install-libwebsockets.sh [--force] [--prefix=/path]

   set -e

   DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
   source "${DIR}/config.sh"

   # Parse arguments
   FORCE=false
   while [[ $# -gt 0 ]]; do
       case $1 in
           --force) FORCE=true; shift ;;
           --prefix=*) DEPS_INSTALL_PREFIX="${1#*=}"; shift ;;
           *) echo "Unknown option: $1"; exit 1 ;;
       esac
   done
   # ... rest of script
   # }}}
   ```

2. Implement version checking:
   - Store installed version in `${DEPS_INSTALL_PREFIX}/.libwebsockets-version`
   - Compare with `LIBWEBSOCKETS_VERSION` from config
   - Skip build if versions match (unless --force)

3. Download source:
   - Clone/pull from https://github.com/warmcat/libwebsockets.git
   - Checkout specific tag

4. Configure CMake with minimal options:
   ```bash
   cmake .. \
       -DCMAKE_INSTALL_PREFIX="${DEPS_INSTALL_PREFIX}" \
       -DLWS_WITH_STATIC=ON \
       -DLWS_WITH_SHARED=OFF \
       -DLWS_WITHOUT_TESTAPPS=ON \
       -DLWS_WITHOUT_TEST_SERVER=ON \
       -DLWS_WITHOUT_TEST_CLIENT=ON \
       -DLWS_WITH_SSL=OFF \
       -DLWS_WITH_ZLIB=OFF
   ```

5. Build and install:
   ```bash
   make -j${BUILD_JOBS}
   make install
   ```

6. Write version file after successful install

## Build Dependencies
- CMake
- C compiler (gcc/clang)
- make

## Related Documents
- 0-001-dependency-management-system.md (parent)
- 2-002-http-server.md (uses libwebsockets)
- 2-003-websocket-handler.md (uses libwebsockets)

## Acceptance Criteria
- [ ] Script downloads correct version
- [ ] Builds successfully without SSL/zlib
- [ ] Installs headers to prefix/include
- [ ] Installs static lib to prefix/lib
- [ ] Version check prevents unnecessary rebuilds
- [ ] --force flag works
- [ ] --prefix flag works
