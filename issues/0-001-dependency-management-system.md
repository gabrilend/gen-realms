# 0-001: Dependency Management System

## Status
**COMPLETE** - 2026-02-11

## Current Behavior
Self-contained dependency management system implemented:
- Install scripts for libwebsockets and libssh in `scripts/deps/`
- Meta-installer `scripts/install-deps.sh` for one-command setup
- Makefile detects local deps first, falls back to system
- Config file `scripts/deps/config.sh` for customization
- Update checking with `--force` override

## Previous Behavior
External dependencies (libwebsockets, libssh) must be installed via system package
manager, creating friction for new developers and limiting portability.

## Intended Behavior
A self-contained dependency management system that:
- Compiles dependencies from source into `libs/` directory
- Prefers local dependencies over system-installed ones
- Provides per-dependency install scripts with update checking
- Offers a meta-install script for one-command project setup
- Allows custom install locations via configuration
- Supports --force flag to override update checks

## Sub-Issues

| ID | Description | Status |
|----|-------------|--------|
| 0-001a | libwebsockets install script | COMPLETE |
| 0-001b | libssh install script | COMPLETE |
| 0-001c | Meta-install and config system | COMPLETE |
| 0-001d | Makefile local dependency priority | COMPLETE |

## Dependencies Requiring Install Scripts

### libwebsockets
- **Source:** https://github.com/warmcat/libwebsockets
- **Version:** v4.3.3 (stable)
- **Build:** CMake
- **Used by:** 02-http.c, 05-websocket.c

### libssh
- **Source:** https://git.libssh.org/projects/libssh.git
- **Version:** 0.10.6 (stable)
- **Build:** CMake
- **Used by:** 03-ssh.c

### cJSON (already local)
- **Source:** Already in libs/cJSON.c
- **Status:** No install script needed

## Directory Structure

```
libs/
├── cJSON.c                    # Already present
├── cJSON.h                    # Already present
├── local/                     # Compiled dependencies
│   ├── include/               # Headers
│   │   ├── libwebsockets.h
│   │   └── libssh/
│   └── lib/                   # Static libraries
│       ├── libwebsockets.a
│       └── libssh.a
└── src/                       # Source downloads (gitignored)
    ├── libwebsockets/
    └── libssh/

scripts/
├── deps/
│   ├── install-libwebsockets.sh
│   ├── install-libssh.sh
│   └── config.sh              # Dependency configuration
└── install-deps.sh            # Meta-installer
```

## Configuration File Format

```bash
# scripts/deps/config.sh

# Where to install compiled dependencies
# Default: ${PROJECT_ROOT}/libs/local
DEPS_INSTALL_PREFIX="${PROJECT_ROOT}/libs/local"

# Where to download source
# Default: ${PROJECT_ROOT}/libs/src
DEPS_SOURCE_DIR="${PROJECT_ROOT}/libs/src"

# Versions to install
LIBWEBSOCKETS_VERSION="v4.3.3"
LIBSSH_VERSION="0.10.6"

# Build options
BUILD_JOBS=$(nproc)
```

## Install Script Interface

Each dependency script should:
1. Accept `--force` to skip update check
2. Accept `--prefix=/path` to override install location
3. Check if already installed and current version
4. Download source if not present
5. Build with appropriate options
6. Install to prefix
7. Print status messages

```bash
# Example usage:
./scripts/deps/install-libwebsockets.sh
./scripts/deps/install-libwebsockets.sh --force
./scripts/deps/install-libwebsockets.sh --prefix=/opt/symbeline/deps
```

## Makefile Integration

The Makefile should:
1. Check for local deps first: `libs/local/lib/`
2. Fall back to system deps if local not found
3. Set include paths appropriately
4. Link statically when using local deps (portability)

## Acceptance Criteria
- [x] libwebsockets compiles from source (scripts/deps/install-libwebsockets.sh)
- [x] libssh compiles from source (scripts/deps/install-libssh.sh)
- [x] Meta-installer works with single command (scripts/install-deps.sh)
- [x] Makefile prefers local over system deps (LOCAL_DEPS_PREFIX detection)
- [x] Config file allows custom install location (SYMBELINE_DEPS_PREFIX)
- [x] Update checking works correctly (version files in libs/local/)
- [x] --force flag overrides checks (passed through to installers)
