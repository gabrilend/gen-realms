# 0-001b: libssh Install Script

## Parent Issue
0-001: Dependency Management System

## Current Behavior
libssh must be installed via system package manager.

## Intended Behavior
Shell script that:
- Downloads libssh source from official git
- Compiles with CMake (static library)
- Installs to local prefix
- Checks for updates before rebuilding
- Supports --force and --prefix flags

## Suggested Implementation Steps

1. Create `scripts/deps/install-libssh.sh`:
   ```bash
   #!/bin/bash
   # {{{ install-libssh.sh
   # Downloads and compiles libssh from source.
   # Usage: ./install-libssh.sh [--force] [--prefix=/path]

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
   - Store installed version in `${DEPS_INSTALL_PREFIX}/.libssh-version`
   - Compare with `LIBSSH_VERSION` from config
   - Skip build if versions match (unless --force)

3. Download source:
   - Clone/pull from https://git.libssh.org/projects/libssh.git
   - Checkout specific tag (libssh-0.10.6)

4. Configure CMake with static build:
   ```bash
   cmake .. \
       -DCMAKE_INSTALL_PREFIX="${DEPS_INSTALL_PREFIX}" \
       -DBUILD_STATIC_LIB=ON \
       -DBUILD_SHARED_LIBS=OFF \
       -DWITH_EXAMPLES=OFF \
       -DWITH_SERVER=ON \
       -DWITH_GCRYPT=OFF \
       -DWITH_MBEDTLS=OFF
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
- OpenSSL development headers (for crypto)

## Note on Crypto Backend
libssh requires a crypto backend. Options:
- OpenSSL (most common, use system OpenSSL)
- gcrypt
- mbedtls

We'll use system OpenSSL since it's widely available and not compile it ourselves.

## Related Documents
- 0-001-dependency-management-system.md (parent)
- 2-004-ssh-server-integration.md (uses libssh)

## Acceptance Criteria
- [ ] Script downloads correct version
- [ ] Builds successfully with system OpenSSL
- [ ] Installs headers to prefix/include/libssh
- [ ] Installs static lib to prefix/lib
- [ ] Version check prevents unnecessary rebuilds
- [ ] --force flag works
- [ ] --prefix flag works
